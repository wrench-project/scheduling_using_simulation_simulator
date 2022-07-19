#include <string>
#include <vector>
#include <simgrid/s4u/NetZone.hpp>

namespace sg4 = simgrid::s4u;

class PlatformCreator {

public:
    PlatformCreator(
            std::string &wms_hostname,
            std::string cluster_specs) : wms_hostname(wms_hostname), cluster_specs(std::move(cluster_specs)) {}

    void operator()() const {
        create_platform();
    }

    static std::tuple<int, int, std::string, std::string, std::string> parseClusterSpecification(std::string spec);


        private:
    void create_platform() const;
    static std::tuple<sg4::NetZone*, simgrid::kernel::routing::NetPoint*, sg4::Link *> create_cluster(std::string name, const sg4::NetZone* root, std::string spec);
    static std::tuple<sg4::NetZone*, simgrid::kernel::routing::NetPoint*, sg4::Link *> create_wms(const sg4::NetZone* root, std::string name, std::string bandwidth);
    std::string wms_hostname;
    std::string cluster_specs;


};


