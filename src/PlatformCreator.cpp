#include "PlatformCreator.h"
#include "SimpleStandardJobScheduler.h"
#include <wrench.h>

std::tuple<sg4::NetZone*, simgrid::kernel::routing::NetPoint*, sg4::Link *>
PlatformCreator::create_wms(const sg4::NetZone* root, std::string name, std::string bandwidth) {
    auto *zone = sg4::create_full_zone("AS_" + name)->set_parent(root);

    // Create the WMSHost host with its disk
    auto wms_host = zone->create_host(name, "100Gf");
    wms_host->set_core_count(1);
    auto wms_host_disk = wms_host->create_disk("hard_drive",
                                               "100MBps",
                                               "100MBps");
    wms_host_disk->set_property("size", "5000GiB");
    wms_host_disk->set_property("mount", "/");

    auto link = zone->create_link(name+"-link", bandwidth)->set_latency("20us");
    return std::make_tuple(zone, wms_host->get_netpoint(), link);
}


std::tuple<sg4::NetZone*, simgrid::kernel::routing::NetPoint*, sg4::Link *>
PlatformCreator::create_cluster(const sg4::NetZone* root, std::string spec) {
    {

        auto parsed_spec = PlatformCreator::parseClusterSpecification(spec);
        std::string name = std::get<0>(parsed_spec);
        int num_hosts = std::get<1>(parsed_spec);
        int num_cores = std::get<2>(parsed_spec);
        std::string flops = std::get<3>(parsed_spec);
        std::string bandwidth = std::get<4>(parsed_spec);

        auto* cluster      = sg4::create_star_zone(name);
        cluster->set_parent(root);

        /* create the backbone link */
        const sg4::Link* l_bb = cluster->create_link("backbone-" + name, "100Gbps")->seal();
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
            auto host = cluster->create_host(hostname, flops);
            host->set_core_count(num_cores);

            /* Create disks on the head host */
            if (i == -1) {
                auto disk1 = host->create_disk(name + "-fs",
                                               "100MBps",
                                               "100MBps");
                disk1->set_property("size", "5000GiB");
                disk1->set_property("mount", "/");
                auto disk2 = host->create_disk(name + "-scratch",
                                               "100MBps",
                                               "100MBps");
                disk2->set_property("size", "5000GiB");
                disk2->set_property("mount", "/scratch");
            }

            /* create UP/DOWN link */
            const sg4::Link* link = cluster->create_split_duplex_link(hostname, "100GBps")->set_latency("10us")->seal();

            /* add link and backbone for communications from the host */
            cluster->add_route(host->get_netpoint(), nullptr, nullptr, nullptr,
                               {{link, sg4::LinkInRoute::Direction::UP}, backbone}, true);
        }

        /* create router */
        auto* router = cluster->create_router(name + "-router");

        auto link = cluster->create_link(name+"-link", bandwidth)->set_latency("20us");
        cluster->seal();

        return std::make_tuple(cluster, router, link);
    }
}

void PlatformCreator::create_platform() const {
    // Create the top-level zone
    auto zone = sg4::create_full_zone("AS0");

    std::vector<std::tuple<sg4::NetZone*, simgrid::kernel::routing::NetPoint*, sg4::Link *>> zones;

    // Create the WMS zone
    zones.push_back(create_wms(zone, "wms_host", "100MBps"));

    // Create all the clusters
    for (auto const &cluster_spec : this->cluster_specs) {
        // Create all cluster hosts
//        std::pair<sg4::NetZone *, simgrid::kernel::routing::NetPoint *> cluster;
        zones.push_back(PlatformCreator::create_cluster(zone, cluster_spec));
    }

    // Create all routes
    for (int i=0; i < zones.size(); i++) {
        auto src = zones.at(i);
        sg4::NetZone* src_zone = std::get<0>(src);
        simgrid::kernel::routing::NetPoint* src_netpoint = std::get<1>(src);
        sg4::Link *src_link = std::get<2>(src);
        for (int j=i+1 ; j < zones.size(); j++) {
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



std::tuple<std::string, int, int, std::string, std::string> PlatformCreator::parseClusterSpecification(std::string spec) {
    auto tokens = SimpleStandardJobScheduler::stringSplit(spec, ':');

    if (tokens.size() != 5) {
        throw std::invalid_argument("Invalid cluster specification '" + spec + "'");
    }
    auto name = tokens.at(0);
    int num_hosts;
    int num_cores;
    try {
        num_hosts = std::stoi(tokens.at(1));
        num_cores = std::stoi(tokens.at(2));
    } catch (std::invalid_argument &e) {
        throw std::invalid_argument("Invalid cluster specification '" + spec + "'");
    }
    auto flops = tokens.at(3);
    auto bandwidth = tokens.at(4);
    return std::make_tuple(name, num_hosts, num_cores, flops, bandwidth);
}