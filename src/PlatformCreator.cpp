#include "PlatformCreator.h"
#include "SimpleStandardJobScheduler.h"
#include <wrench.h>
#include <stdlib.h>

std::tuple<sg4::NetZone*, simgrid::kernel::routing::NetPoint*, sg4::Link *>
PlatformCreator::create_wms(const sg4::NetZone* root, std::string name, std::string bandwidth) {
    auto *zone = sg4::create_full_zone("AS_" + name)->set_parent(root);

    // Create the WMSHost host with its disk
    auto wms_host = zone->create_host(name, "100Gf");
    wms_host->set_core_count(1);
    auto wms_host_disk = wms_host->create_disk("hard_drive",
                                               "100MBps",
                                               "100MBps");
    wms_host_disk->set_read_bandwidth(this->bandwidth_factor * wms_host_disk->get_read_bandwidth());
    wms_host_disk->set_write_bandwidth(this->bandwidth_factor * wms_host_disk->get_write_bandwidth());
    wms_host_disk->set_property("size", "5000EiB");
    wms_host_disk->set_property("mount", "/");

    auto link = zone->create_link(name+"-link", bandwidth)->set_latency("20us");
    return std::make_tuple(zone, wms_host->get_netpoint(), link);
}


std::tuple<sg4::NetZone*, simgrid::kernel::routing::NetPoint*, sg4::Link *>
PlatformCreator::create_cluster(const std::string name, const sg4::NetZone* root, std::string spec) {

    auto parsed_spec = PlatformCreator::parseClusterSpecification(spec);
    int num_hosts = std::get<0>(parsed_spec);
    int num_cores = std::get<1>(parsed_spec);
    std::string flops = std::get<2>(parsed_spec);
    std::string watts = std::get<3>(parsed_spec);
    std::string io_bandwidth = std::get<4>(parsed_spec);
    std::string internet_bandwidth = std::get<5>(parsed_spec);

    auto* cluster      = sg4::create_star_zone(name);
    cluster->set_parent(root);

    /* create the backbone link, which is the one on which all disk reads/writes will bottleneck */
    /* We do it this way so that we can set its sharing policy to FATPIPE when using the --no-contention flag */
    auto l_bb = cluster->create_link("backbone-" + name, io_bandwidth)->seal();
    l_bb->set_sharing_policy(simgrid::s4u::Link::SharingPolicy::SHARED); // probably redundant
    sg4::LinkInRoute backbone(l_bb);

    /* create all hosts and connect them to outside world */
    for (int i=-1; i < num_hosts; i++) {
        std::string hostname;
        if (i == -1) {
            hostname = name + "-head";
        } else {
            hostname = name + "-node-" + std::to_string(i);
        }
        /* create host */
        double flop_number = atof(flops.c_str()); // Will remove the units!
        std::string flop_unit;
        for (char c : flops) {
            if (((c >= '0') and (c <= '9')) or (c == '.')) {
                continue;
            }
            flop_unit += c;
        }

        std::vector<std::string> speed_per_pstate;
        int half_num_pstates = 100;
        for (int j=1; j < 2*half_num_pstates; j++) {
            double pstate_speed = flop_number * (1.0 - (double)(half_num_pstates - j)/half_num_pstates);
            speed_per_pstate.push_back(std::to_string(pstate_speed)+flop_unit);
        }

        auto host = cluster->create_host(hostname, speed_per_pstate);

        host->set_core_count(num_cores);

        // Creating pstates as a trick to make it possible to inject platform noise (i.e., wrong flop rates)
        std::string wattage_per_state_value;
        for (unsigned long j=0; j < speed_per_pstate.size(); j++) {
            wattage_per_state_value += std::string("10.00:" + watts) + (j < speed_per_pstate.size() - 1 ? "," : "");
        }

        host->set_property("wattage_per_state", wattage_per_state_value);
        host->set_property("wattage_off", "0.0");

        /* Create disks on the head host */
        /* Note that the bandwidth is huge. This is because I/O will bottleneck
         * on the backbone link, for which we can set the sharing policy to FAT_PIPE
         * so as to implement the --no-contention option!
         */
        if (i == -1) {
            auto disk1 = host->create_disk(name + "-fs",
                                           "1000TBps",
                                           "1000TBps");
            disk1->set_property("size", "5000EiB");
            disk1->set_property("mount", "/");
            auto disk2 = host->create_disk(name + "-scratch",
                                           "1000TBps",
                                           "1000TBps");
            disk2->set_property("size", "5000EiB");
            disk2->set_property("mount", "/scratch");
        }

        /* create UP/DOWN link */
        const sg4::Link* link = cluster->create_split_duplex_link(hostname, "100Gbps")->set_latency("10us")->seal();

        /* add link and backbone for communications from the host */
        cluster->add_route(host->get_netpoint(), nullptr, nullptr, nullptr,
                           {{link, sg4::LinkInRoute::Direction::UP}, backbone}, true);
    }

    /* create router */
    auto* router = cluster->create_router(name + "-router");

    auto link = cluster->create_link(name+"-link", internet_bandwidth)->set_latency("20us");
    cluster->seal();

    return std::make_tuple(cluster, router, link);
}

void PlatformCreator::create_platform() {
    // Create the top-level zone
    auto zone = sg4::create_full_zone("AS0");

    std::vector<std::tuple<sg4::NetZone*, simgrid::kernel::routing::NetPoint*, sg4::Link *>> zones;

    // Create the WMS zone
    zones.push_back(this->create_wms(zone, "wms_host", "100MBps"));

    // Split the cluster specs into individual specs
    auto tokens = SimpleStandardJobScheduler::stringSplit(this->cluster_specs, ',');

    // Create all the clusters
    int counter = 0;
    for (auto const &cluster_spec : tokens) {
        // Create all cluster hosts
//        std::pair<sg4::NetZone *, simgrid::kernel::routing::NetPoint *> cluster;
        zones.push_back(PlatformCreator::create_cluster(std::string("cluster_") + std::to_string(counter++), zone, cluster_spec));
    }

    // Create all routes
    for (unsigned long i=0; i < zones.size(); i++) {
        auto src = zones.at(i);
        sg4::NetZone* src_zone = std::get<0>(src);
        simgrid::kernel::routing::NetPoint* src_netpoint = std::get<1>(src);
        sg4::Link *src_link = std::get<2>(src);
        for (unsigned long j=i+1 ; j < zones.size(); j++) {
            auto dst = zones.at(j);
            sg4::NetZone* dst_zone = std::get<0>(dst);
            simgrid::kernel::routing::NetPoint* dst_netpoint = std::get<1>(dst);
            sg4::Link *dst_link = std::get<2>(dst);

            if (src != dst) {
                sg4::LinkInRoute link1{src_link};
                sg4::LinkInRoute link2{dst_link};
                zone->add_route(
                        src_zone->get_netpoint(),
                        dst_zone->get_netpoint(),
                        src_netpoint,
                        dst_netpoint,
                        {link1, link2});
            }
        }
    }

    // Apply the bandwidth factor to all links
    for (const auto &link : zone->get_impl()->get_all_links()) {
        link->set_bandwidth(this->bandwidth_factor * link->get_bandwidth());
    }

    zone->seal();

//    auto hostnames = wrench::Simulation::getHostnameList();
//    for (auto const &src : hostnames) {
//        for (auto const &dst : hostnames) {
//            std::vector<simgrid::s4u::Link *> links;
//            double stuff;
//            simgrid::s4u::Host::by_name(src)->route_to(simgrid::s4u::Host::by_name(dst), links, &stuff);
//            std::cerr << src << "---> " << dst << "\n";
//            for (const auto &l : links) {
//                std::cerr << "LINK " << l->get_name() << "\n";
//            }
//        }
//    }

}



std::tuple<int, int, std::string, std::string, std::string, std::string> PlatformCreator::parseClusterSpecification(const std::string& spec) {
    auto tokens = SimpleStandardJobScheduler::stringSplit(spec, ':');

    if (tokens.size() != 6) {
        throw std::invalid_argument("Invalid cluster specification '" + spec + "'");
    }
    int num_hosts;
    int num_cores;
    try {
        num_hosts = std::stoi(tokens.at(0));
        num_cores = std::stoi(tokens.at(1));
    } catch (std::invalid_argument &e) {
        throw std::invalid_argument("Invalid cluster specification '" + spec + "'");
    }
    auto flops = tokens.at(2);
    auto watts = tokens.at(3);
    auto io_bandwidth = tokens.at(4);
    auto internet_bandwidth = tokens.at(5);
    return std::make_tuple(num_hosts, num_cores, flops, watts, io_bandwidth, internet_bandwidth);
}
