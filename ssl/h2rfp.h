#ifndef TC_SSL_H2RFP_H
#define TC_SSL_H2RFP_H

#include "./network.h"
#include "../protocols/h2rfp.h"

namespace tobilib {
namespace h2rfp {
    using SSL_Endpoint = Endpoint<network::SSL_Endpoint>;
    using WSS_Endpoint = Endpoint<network::WSS_Endpoint>;

}
}
        
#endif