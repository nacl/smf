// Copyright (c) 2016 Alexander Gallego. All rights reserved.
//
#include "smf/histogram.h"

#include <cstdlib>
#include <hdr_histogram.h>
#include <hdr_histogram_log.h>

#include <iostream>

namespace smf {
seastar::lw_shared_ptr<histogram>
histogram::make_lw_shared(int64_t max_value) {
  auto x = seastar::make_lw_shared<histogram>(max_value);
  assert(x->hist_->hist);
  return x;
}
std::unique_ptr<histogram>
histogram::make_unique(int64_t max_value) {
  std::unique_ptr<histogram> p(new histogram(max_value));
  assert(p->hist_->hist);
  return p;
}

histogram::histogram(int64_t max_value)
  : hist_(std::make_unique<hist_t>(max_value)) {}
histogram::histogram(histogram &&o) noexcept : hist_(std::move(o.hist_)) {}

histogram &
histogram::operator+=(const histogram &o) {
  ::hdr_add(hist_->hist, o.hist_->hist);
  return *this;
}
histogram &
histogram::operator+=(const hist_t *o) {
  ::hdr_add(hist_->hist, o->hist);
  return *this;
}

histogram &
histogram::operator=(histogram &&o) noexcept {
  hist_ = std::move(o.hist_);
  return *this;
}

void
histogram::record(const uint64_t &v) {
  ::hdr_record_value(hist_->hist, v);
}

void
histogram::record_multiple_times(const uint64_t &v, const uint32_t &times) {
  ::hdr_record_values(hist_->hist, v, times);
}

void
histogram::record_corrected(const uint64_t &v, const uint64_t &interval) {
  ::hdr_record_corrected_value(hist_->hist, v, interval);
}

int64_t
histogram::value_at(double percentile) const {
  return ::hdr_value_at_percentile(hist_->hist, percentile);
}
double
histogram::stddev() const {
  return ::hdr_stddev(hist_->hist);
}
double
histogram::mean() const {
  return ::hdr_mean(hist_->hist);
}
size_t
histogram::memory_size() const {
  return ::hdr_get_memory_size(hist_->hist);
}

hdr_histogram *
histogram::get() {
  assert(hist_);
  return hist_->hist;
}

std::unique_ptr<histogram_measure>
histogram::auto_measure() {
  return std::make_unique<histogram_measure>(shared_from_this());
}

int
histogram::print(FILE *fp) const {
  assert(fp != nullptr);
  return ::hdr_percentiles_print(hist_->hist,
                                 fp,        // File to write to
                                 5,         // Granularity of printed values
                                 1.0,       // Multiplier for results
                                 CLASSIC);  // Format CLASSIC/CSV supported.
}

histogram::~histogram() {}

}  // namespace smf
