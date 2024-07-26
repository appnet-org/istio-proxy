#include <netinet/in.h>
#include <string>
#include <chrono>
#include <mutex>

#include "ratelimit_filter.h"
#include "source/extensions/filters/http/ratelimit/echo.pb.h"

#include "envoy/server/filter_config.h"
#include "google/protobuf/extension_set.h"
#include "source/common/http/utility.h"
#include "source/common/http/message_impl.h" 
#include "envoy/upstream/resource_manager.h"


namespace Envoy {
namespace Http {



// New rate limiting related members

std::mutex rate_limit_mutex;
bool init = false;
std::chrono::time_point<std::chrono::steady_clock> last_ts;
double token_bucket_size;
double token_bucket;
double per_sec_rate;


RatelimitFilterConfig::RatelimitFilterConfig(
  const sample::FilterConfig&, Envoy::Server::Configuration::FactoryContext &ctx)
  : ctx_(ctx) { }

RatelimitFilter::RatelimitFilter(RatelimitFilterConfigSharedPtr config)
  : config_(config), empty_callback_(new EmptyCallback{}) {

  std::unique_lock<std::mutex> lock(rate_limit_mutex);
  if (!init) {
    last_ts = std::chrono::steady_clock::now();
    token_bucket_size = 5.0;
    token_bucket = token_bucket_size;
    per_sec_rate = 1.0;
    init = true;
  }
}

RatelimitFilter::~RatelimitFilter() {}

void RatelimitFilter::onDestroy() {}

bool RatelimitFilter::rateLimitCheck() {
  std::unique_lock<std::mutex> lock(rate_limit_mutex);
  auto now = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed = now - last_ts;
  double elapsed_seconds = elapsed.count();
  token_bucket = std::min(token_bucket_size, token_bucket + per_sec_rate * elapsed_seconds);
  last_ts = now;

  ENVOY_LOG(info, "Rate limit check: limit_rate={}, token_rate={}, per_sec_rate={}, elapsed_seconds={}", token_bucket_size, token_bucket, per_sec_rate, elapsed_seconds);
  if (token_bucket > 1.0) {
    token_bucket -= 1.0;
    // print all states
    return true;
  } else {
    return false;
  }
}

FilterHeadersStatus RatelimitFilter::decodeHeaders(RequestHeaderMap&, bool) {
  return FilterHeadersStatus::StopIteration;
}

FilterDataStatus RatelimitFilter::decodeData(Buffer::Instance&, bool end_of_stream) {
  if (!end_of_stream) 
    return FilterDataStatus::Continue;
  if (!rateLimitCheck()) {
    ENVOY_LOG(info, "Rate limit exceeded");
    decoder_callbacks_->sendLocalReply(Http::Code::TooManyRequests, "Rate limit exceeded", nullptr, absl::nullopt, "");
    return FilterDataStatus::StopIterationNoBuffer;
  } else {
    return FilterDataStatus::Continue;
  }
}

void RatelimitFilter::setDecoderFilterCallbacks(StreamDecoderFilterCallbacks& callbacks) {
  decoder_callbacks_ = &callbacks;
}

void RatelimitFilter::setEncoderFilterCallbacks(StreamEncoderFilterCallbacks& callbacks) {
  encoder_callbacks_ = &callbacks;
}

FilterHeadersStatus RatelimitFilter::encodeHeaders(ResponseHeaderMap& headers, bool) {
  ENVOY_LOG(info, "[Ratelimit Filter] encodeHeaders {}", headers);
  return FilterHeadersStatus::Continue;
}

FilterDataStatus RatelimitFilter::encodeData(Buffer::Instance &, bool) {
  return FilterDataStatus::Continue;
}

void RatelimitFilter::onSuccess(const Http::AsyncClient::Request&,
                 Http::ResponseMessagePtr&&) {
}

void RatelimitFilter::onFailure(const Http::AsyncClient::Request&,
                 Http::AsyncClient::FailureReason) {
}

void RatelimitFilter::onBeforeFinalizeUpstreamSpan(Tracing::Span&,
                          const Http::ResponseHeaderMap*) {
  ENVOY_LOG(info, "[Ratelimit Filter] ExternalResponseCallback onBeforeFinalizeUpstreamSpan");
}

bool RatelimitFilter::sendWebdisRequest(const std::string path, Callbacks &callback) {
  auto cluster = this->config_->ctx_.serverFactoryContext().clusterManager().getThreadLocalCluster("webdis_cluster");
  if (!cluster) {
  ENVOY_LOG(info, "webdis_cluster not found");
  assert(0);
  return false;
  }
  Http::RequestMessagePtr request = std::make_unique<Http::RequestMessageImpl>();

  request->headers().setMethod(Http::Headers::get().MethodValues.Get);
  request->headers().setHost("localhost:7379");
  ENVOY_LOG(info, "[Ratelimit Filter] webdis requesting path={}", path);
  request->headers().setPath(path);
  auto options = Http::AsyncClient::RequestOptions()
           .setTimeout(std::chrono::milliseconds(1000))
           .setSampled(absl::nullopt);
  cluster->httpAsyncClient().send(std::move(request), callback, options);
  return true;
}

} // namespace Http
} // namespace Envoy
