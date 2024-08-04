#include "source/extensions/load_balancing_policies/random/random_lb.h"

namespace Envoy {
namespace Upstream {

HostConstSharedPtr MyRandomLoadBalancer::peekAnotherHost(LoadBalancerContext* context) {
  if (tooManyPreconnects(stashed_random_.size(), total_healthy_hosts_)) {
    return nullptr;
  }
  return peekOrChoose(context, true);
}

HostConstSharedPtr MyRandomLoadBalancer::chooseHostOnce(LoadBalancerContext* context) {
  return peekOrChoose(context, false);
}

HostConstSharedPtr MyRandomLoadBalancer::peekOrChoose(LoadBalancerContext* context, bool peek) {

  ENVOY_LOG(error, "peekOrChoose: peek={}", peek);

  uint64_t random_hash = random(peek);
  const absl::optional<HostsSource> hosts_source = hostSourceToUse(context, random_hash);
  if (!hosts_source) {
    return nullptr;
  }

  const HostVector& hosts_to_use = hostSourceToHosts(*hosts_source);
  if (hosts_to_use.empty()) {
    return nullptr;
  }

  return hosts_to_use[random_hash % hosts_to_use.size()];
}

} // namespace Upstream
} // namespace Envoy
