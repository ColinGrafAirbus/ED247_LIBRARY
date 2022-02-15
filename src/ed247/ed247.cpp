/******************************************************************************
 * The MIT Licence
 *
 * Copyright (c) 2021 Airbus Operations S.A.S
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *****************************************************************************/

/************
 * Includes *
 ************/

#include "ed247.h"
#include "ed247_internals.h"
#include "ed247_logs.h"
#include "ed247_context.h"

#include <memory>

/***********
 * Defines *
 ***********/

#ifdef _PRODUCT_VERSION
const char* _ed247_version = _PRODUCT_VERSION;
#else
const char* _ed247_version = "unversioned";
#endif

#ifdef _PRODUCT_NAME
const char* _ed247_name = _PRODUCT_NAME;
#else
const char* _ed247_name = "unnamed";
#endif

#define LIBED247_CATCH(topic)                                           \
  catch(std::exception &ex)                                             \
  {                                                                     \
    PRINT_ERROR(topic << " failed (FATAL ERROR) " << ex.what());        \
    return ED247_STATUS_FAILURE;                                        \
  }                                                                     \
  catch(...)                                                            \
  {                                                                     \
    PRINT_ERROR(topic << " failed (FATAL ERROR) (unknown exception)");  \
    return ED247_STATUS_FAILURE;                                        \
  }

/************
 * # Common *
 ************/

const char * libed247_errors()
{
  return nullptr;
}

const char * ed247_status_string(
  ed247_status_t status)
{
  switch(status){
  case ED247_STATUS_SUCCESS:          return ed247::defines::status::SUCCESS.c_str();
  case ED247_STATUS_FAILURE:          return ed247::defines::status::FAILURE.c_str();
  case ED247_STATUS_TIMEOUT:          return ed247::defines::status::TIMEOUT.c_str();
  case ED247_STATUS_NODATA:           return ed247::defines::status::NODATA.c_str();
  default:                            return "Unknown";
  }
}

const char * ed247_standard_string(
  ed247_standard_t standard)
{
  switch(standard){
  case ED247_STANDARD_ED247:  return ed247::defines::standard::ED247.c_str();
  case ED247_STANDARD_ED247A: return ed247::defines::standard::ED247A.c_str();
  default:                    return "Unknown";
  }
}

const char * ed247_direction_string(
  ed247_direction_t direction)
{
  switch(direction){
  case ED247_DIRECTION_IN:    return ed247::defines::direction::INPUT.c_str();
  case ED247_DIRECTION_OUT:   return ed247::defines::direction::OUTPUT.c_str();
  case ED247_DIRECTION_INOUT: return ed247::defines::direction::INOUT.c_str();
  default:                    return "Unknown";
  }
}

ed247_standard_t ed247_standard_from_string(
  const char *standard)
{
  using namespace ed247::defines::standard;
  if(ED247.compare(standard) == 0){
    return ED247_STANDARD_ED247;
  }else if(ED247A.compare(standard) == 0){
    return ED247_STANDARD_ED247A;
  }else{
    return ED247_STANDARD__INVALID;
  }
}

ed247_direction_t ed247_direction_from_string(
  const char *direction)
{
  using namespace ed247::defines;
  if(direction::INPUT.compare(direction) == 0){
    return ED247_DIRECTION_IN;
  }else if(direction::OUTPUT.compare(direction) == 0){
    return ED247_DIRECTION_OUT;
  }else if(direction::INOUT.compare(direction) == 0){
    return ED247_DIRECTION_INOUT;
  }else{
    return ED247_DIRECTION__INVALID;
  }
}

const char * ed247_yesno_string(
  ed247_yesno_t yesno)
{
  using namespace ed247::defines::yesno;
  switch(yesno){
  case ED247_YESNO_NO:    return NO.c_str();
  case ED247_YESNO_YES:   return YES.c_str();
  default:                return "Unknown";
  }
}

ed247_yesno_t ed247_yesno_from_string(
  const char *yesno)
{
  using namespace ed247::defines::yesno;
  if(strcmp(yesno,"No") == 0){
    return ED247_YESNO_NO;
  }else if(strcmp(yesno,"Yes") == 0){
    return ED247_YESNO_YES;
  }else{
    return ED247_YESNO__INVALID;
  }
}

const char * ed247_component_type_string(
  ed247_component_type_t component_type)
{
  switch(component_type){
  case ED247_COMPONENT_TYPE_VIRTUAL:  return ed247::defines::component_type::VIRTUAL.c_str();
  case ED247_COMPONENT_TYPE_BRIDGE:   return ed247::defines::component_type::BRIDGE.c_str();
  default:                            return ed247::defines::Unknown.c_str();
  }
}

ed247_component_type_t ed247_component_type_from_string(
  const char *component_type)
{
  using namespace ed247::defines::component_type;
  if(VIRTUAL.compare(component_type) == 0){
    return ED247_COMPONENT_TYPE_VIRTUAL;
  }else if(BRIDGE.compare(component_type) == 0){
    return ED247_COMPONENT_TYPE_BRIDGE;
  }else{
    return ED247_COMPONENT_TYPE__INVALID;
  }
}

const char * ed247_stream_type_string(
  ed247_stream_type_t stream_type)
{
  switch(stream_type){
  case ED247_STREAM_TYPE_A664:       return ed247::defines::stream_type::A664.c_str();
  case ED247_STREAM_TYPE_A429:       return ed247::defines::stream_type::A429.c_str();
  case ED247_STREAM_TYPE_A825:       return ed247::defines::stream_type::A825.c_str();
  case ED247_STREAM_TYPE_M1553:      return ed247::defines::stream_type::M1553.c_str();
  case ED247_STREAM_TYPE_SERIAL:     return ed247::defines::stream_type::SERIAL.c_str();
  case ED247_STREAM_TYPE_AUDIO:      return ed247::defines::stream_type::AUDIO.c_str();
  case ED247_STREAM_TYPE_VIDEO:      return ed247::defines::stream_type::VIDEO.c_str();
  case ED247_STREAM_TYPE_ETHERNET:   return ed247::defines::stream_type::ETHERNET.c_str();
  case ED247_STREAM_TYPE_ANALOG:     return ed247::defines::stream_type::ANALOG.c_str();
  case ED247_STREAM_TYPE_DISCRETE:   return ed247::defines::stream_type::DISCRETE.c_str();
  case ED247_STREAM_TYPE_NAD:        return ed247::defines::stream_type::NAD.c_str();
  case ED247_STREAM_TYPE_VNAD:       return ed247::defines::stream_type::VNAD.c_str();
  default:                        return ed247::defines::Unknown.c_str();
  }
}

ed247_stream_type_t ed247_stream_type_from_string(
  const char *stream_type)
{
  using namespace ed247::defines::stream_type;
  if(A664.compare(stream_type) == 0){
    return ED247_STREAM_TYPE_A664;
  }else if(A429.compare(stream_type) == 0){
    return ED247_STREAM_TYPE_A429;
  }else if(A825.compare(stream_type) == 0){
    return ED247_STREAM_TYPE_A825;
  }else if(M1553.compare(stream_type) == 0){
    return ED247_STREAM_TYPE_M1553;
  }else if(SERIAL.compare(stream_type) == 0){
    return ED247_STREAM_TYPE_SERIAL;
  }else if(AUDIO.compare(stream_type) == 0){
    return ED247_STREAM_TYPE_AUDIO;
  }else if(VIDEO.compare(stream_type) == 0){
    return ED247_STREAM_TYPE_VIDEO;
  }else if(ETHERNET.compare(stream_type) == 0){
    return ED247_STREAM_TYPE_ETHERNET;
  }else if(ANALOG.compare(stream_type) == 0){
    return ED247_STREAM_TYPE_ANALOG;
  }else if(DISCRETE.compare(stream_type) == 0){
    return ED247_STREAM_TYPE_DISCRETE;
  }else if(NAD.compare(stream_type) == 0){
    return ED247_STREAM_TYPE_NAD;
  }else if(VNAD.compare(stream_type) == 0){
    return ED247_STREAM_TYPE_VNAD;
  }else{
    return ED247_STREAM_TYPE__INVALID;
  }
}

const char * ed247_signal_type_string(
  ed247_signal_type_t signal_type)
{
  switch(signal_type){
  case ED247_SIGNAL_TYPE_ANALOG:     return ed247::defines::signal_type::ANALOG.c_str();
  case ED247_SIGNAL_TYPE_DISCRETE:   return ed247::defines::signal_type::DISCRETE.c_str();
  case ED247_SIGNAL_TYPE_NAD:        return ed247::defines::signal_type::NAD.c_str();
  case ED247_SIGNAL_TYPE_VNAD:       return ed247::defines::signal_type::VNAD.c_str();
  default:                        return ed247::defines::Unknown.c_str();
  }
}

ed247_signal_type_t ed247_signal_type_from_string(
  const char *signal_type)
{
  using namespace ed247::defines::signal_type;
  if(ANALOG.compare(signal_type) == 0){
    return ED247_SIGNAL_TYPE_ANALOG;
  }else if(DISCRETE.compare(signal_type) == 0){
    return ED247_SIGNAL_TYPE_DISCRETE;
  }else if(NAD.compare(signal_type) == 0){
    return ED247_SIGNAL_TYPE_NAD;
  }else if(VNAD.compare(signal_type) == 0){
    return ED247_SIGNAL_TYPE_VNAD;
  }else{
    return ED247_SIGNAL_TYPE__INVALID;
  }
}

const char * ed247_nad_type_string(
  ed247_nad_type_t nad_type)
{
  switch(nad_type){
  case ED247_NAD_TYPE_INT8:       return ed247::defines::nad_type::INT8.c_str();
  case ED247_NAD_TYPE_INT16:      return ed247::defines::nad_type::INT16.c_str();
  case ED247_NAD_TYPE_INT32:      return ed247::defines::nad_type::INT32.c_str();
  case ED247_NAD_TYPE_INT64:      return ed247::defines::nad_type::INT64.c_str();
  case ED247_NAD_TYPE_UINT8:      return ed247::defines::nad_type::UINT8.c_str();
  case ED247_NAD_TYPE_UINT16:     return ed247::defines::nad_type::UINT16.c_str();
  case ED247_NAD_TYPE_UINT32:     return ed247::defines::nad_type::UINT32.c_str();
  case ED247_NAD_TYPE_UINT64:     return ed247::defines::nad_type::UINT64.c_str();
  case ED247_NAD_TYPE_FLOAT32:    return ed247::defines::nad_type::FLOAT32.c_str();
  case ED247_NAD_TYPE_FLOAT64:    return ed247::defines::nad_type::FLOAT64.c_str();
  default:                        return ed247::defines::Unknown.c_str();
  }
}

ed247_nad_type_t ed247_nad_type_from_string(
  const char *nad_type)
{
  using namespace ed247::defines;
  if(nad_type::INT8.compare(nad_type) == 0){
    return ED247_NAD_TYPE_INT8;
  }else if(nad_type::INT16.compare(nad_type) == 0){
    return ED247_NAD_TYPE_INT16;
  }else if(nad_type::INT32.compare(nad_type) == 0){
    return ED247_NAD_TYPE_INT32;
  }else if(nad_type::INT64.compare(nad_type) == 0){
    return ED247_NAD_TYPE_INT64;
  }else if(nad_type::UINT8.compare(nad_type) == 0){
    return ED247_NAD_TYPE_UINT8;
  }else if(nad_type::UINT16.compare(nad_type) == 0){
    return ED247_NAD_TYPE_UINT16;
  }else if(nad_type::UINT32.compare(nad_type) == 0){
    return ED247_NAD_TYPE_UINT32;
  }else if(nad_type::UINT64.compare(nad_type) == 0){
    return ED247_NAD_TYPE_UINT64;
  }else if(nad_type::FLOAT32.compare(nad_type) == 0){
    return ED247_NAD_TYPE_FLOAT32;
  }else if(nad_type::FLOAT64.compare(nad_type) == 0){
    return ED247_NAD_TYPE_FLOAT64;
  }else{
    return ED247_NAD_TYPE__INVALID;
  }
}

size_t ed247_nad_type_size(
  ed247_nad_type_t nad_type)
{
  return ed247::BaseSignal::nad_type_size(nad_type);
}

/*********
 * Lists *
 *********/

ed247_status_t ed247_channel_list_next(
  ed247_channel_list_t channels,
  ed247_channel_t *channel)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!channels) {
    PRINT_ERROR(__func__ << ": invalid channels list.");
    return ED247_STATUS_FAILURE;
  }
  if(!channel) {
    PRINT_ERROR(__func__ << ": channel pointer is NULL.");
    return ED247_STATUS_FAILURE;
  }
  *channel = nullptr;
  try{
    auto ed247_channels = static_cast<ed247::SmartListChannels*>(channels);
    auto && next = ed247_channels->next_ok();
    *channel = next ? next->get() : nullptr;
  }
  LIBED247_CATCH("Channel list next");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_channel_list_free(
  ed247_channel_list_t channels)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!channels) {
    PRINT_ERROR(__func__ << ": invalid channels list.");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_channels = static_cast<ed247::SmartListChannels*>(channels);
    if(!ed247_channels->managed()) {
      delete ed247_channels;
    }
  }
  LIBED247_CATCH("Channel list free");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_channel_list_size(
  ed247_channel_list_t channels,
  size_t * size)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!channels) {
    PRINT_ERROR(__func__ << ": invalid channels list.");
    return ED247_STATUS_FAILURE;
  }
  if(!size) {
    PRINT_ERROR(__func__ << ": Invalid size pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_channels = static_cast<ed247::SmartListChannels*>(channels);
    *size = ed247_channels->size();
  }
  LIBED247_CATCH("Channel list size");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_list_next(
  ed247_stream_list_t streams,
  ed247_stream_t *stream)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!streams) {
    PRINT_ERROR(__func__ << ": Invalid streams");
    return ED247_STATUS_FAILURE;
  }
  if(!stream) {
    PRINT_ERROR(__func__ << ": Empty stream pointer");
    return ED247_STATUS_FAILURE;
  }
  *stream = nullptr;
  try{
    auto ed247_streams = static_cast<ed247::SmartListStreams*>(streams);
    auto && next = ed247_streams->next_ok();
    *stream = next ? next->get() : nullptr;
  }
  LIBED247_CATCH("Stream list next");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_list_free(
  ed247_stream_list_t streams)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!streams) {
    PRINT_ERROR(__func__ << ": Invalid streams");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_streams = static_cast<ed247::SmartListStreams*>(streams);
    if(!ed247_streams->managed())
      delete ed247_streams;
  }
  LIBED247_CATCH("Stream list free");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_list_size(
  ed247_stream_list_t streams,
  size_t * size)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!streams) {
    PRINT_ERROR(__func__ << ": Invalid streams");
    return ED247_STATUS_FAILURE;
  }
  if(!size) {
    PRINT_ERROR(__func__ << ": Invalid size pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_streams = static_cast<ed247::SmartListStreams*>(streams);
    *size = ed247_streams->size();
  }
  LIBED247_CATCH("Stream list size");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_signal_list_next(
  ed247_signal_list_t signals,
  ed247_signal_t *signal)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!signals) {
    PRINT_ERROR(__func__ << ": Invalid signals");
    return ED247_STATUS_FAILURE;
  }
  if(!signal) {
    PRINT_ERROR(__func__ << ": Empty signal pointer");
    return ED247_STATUS_FAILURE;
  }
  *signal = nullptr;
  try{
    auto ed247_signals = (ed247::SmartListSignals*)(signals);
    auto && next = ed247_signals->next_ok();
    *signal = next ? next->get() : nullptr;
  }
  LIBED247_CATCH("Signal list next");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_signal_list_free(
  ed247_signal_list_t signals)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!signals) {
    PRINT_ERROR(__func__ << ": Invalid signals");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_signals = static_cast<ed247::SmartListSignals*>(signals);
    if(!ed247_signals->managed()) {
      delete ed247_signals;
    }
  }
  LIBED247_CATCH("Signal list free");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_signal_list_size(
  ed247_signal_list_t signals,
  size_t * size)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!signals) {
    PRINT_ERROR(__func__ << ": Invalid signals");
    return ED247_STATUS_FAILURE;
  }
  if(!size) {
    PRINT_ERROR(__func__ << ": Invalid size pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_signals = static_cast<ed247::SmartListSignals*>(signals);
    *size = ed247_signals->size();
  }
  LIBED247_CATCH("Signal list size");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_frame_list_next(
  ed247_frame_list_t frames,
  const ed247_frame_t ** frame)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!frames) {
    PRINT_ERROR(__func__ << ": Invalid frames");
    return ED247_STATUS_FAILURE;
  }
  if(!frame) {
    PRINT_ERROR(__func__ << ": Empty frame pointer");
    return ED247_STATUS_FAILURE;
  }
  *frame = nullptr;
  try {
    ed247::SmartListFrames* ed247_frames = (ed247::SmartListFrames*)(frames);
    auto && next = ed247_frames->next_ok();
    *frame = next ? next->get() : nullptr;
  }
  LIBED247_CATCH("Frame list next");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_frame_list_free(
  ed247_frame_list_t frames)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!frames) {
    PRINT_ERROR(__func__ << ": Invalid frames");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_frames = static_cast<ed247::SmartListFrames*>(frames);
    if(!ed247_frames->managed())
      delete ed247_frames;
  }
  LIBED247_CATCH("Frame list free");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_frame_list_size(
  ed247_frame_list_t frames,
  size_t * size)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!frames) {
    PRINT_ERROR(__func__ << ": Invalid frames");
    return ED247_STATUS_FAILURE;
  }
  if(!size) {
    PRINT_ERROR(__func__ << ": Invalid size pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_frames = static_cast<ed247::SmartListFrames*>(frames);
    *size = ed247_frames->size();
  }
  LIBED247_CATCH("Frame list size");
  return ED247_STATUS_SUCCESS;
}

/******************
 * Initialization *
 ******************/

const char * ed247_get_implementation_name()
{
  return _ed247_name;
}

const char * ed247_get_implementation_version()
{
  return _ed247_version;
}

ed247_status_t ed247_get_runtime_metrics(
  ed247_context_t context,
  const libed247_runtime_metrics_t ** metrics)
{
  if(!metrics) {
    PRINT_ERROR(__func__ << ": Invalid metrics pointer");
    return ED247_STATUS_FAILURE;
  }
  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    *metrics = NULL;
    auto ed247_context = static_cast<ed247::Context*>(context);
    *metrics = ed247_context->get_runtime_metrics();
  }
  LIBED247_CATCH("Get metrics");
  return ED247_STATUS_SUCCESS;
}

extern LIBED247_EXPORT ed247_status_t ed247_set_log(
  ed247_log_level_t log_level,
  const char* log_filepath)
{
  ed247::log::get().reset(log_level, log_filepath);
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_set_log_level(
  ed247_log_level_t log_level)
{
  ed247::log::get().reset(log_level, nullptr);
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_get_log_level(
  ed247_log_level_t *log_level)
{
  *log_level = ed247::log::get().level();
  return ED247_STATUS_SUCCESS;
}

ed247_status_t libed247_register_set_simulation_time_ns_handler(
  libed247_set_simulation_time_ns_t handler,
  void *user_data)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!handler) {
    PRINT_ERROR(__func__ << ": Invalid handler");
    return ED247_STATUS_FAILURE;
  }
  try {
    ed247::SimulationTimeHandler::get().set_handler(handler, user_data);
  }
  LIBED247_CATCH("Register simulation time handler");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t libed247_set_simulation_time_ns(
  ed247_time_sample_t time_sample,
  void *user_data)
{
  _UNUSED(user_data);
  if(!time_sample) {
      PRINT_ERROR(__func__ << ": invalid time_sample pointer");
      return ED247_STATUS_FAILURE;
  }
#ifdef __unix__
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
  time_sample->epoch_s = (uint32_t)tp.tv_sec;
  time_sample->offset_ns = (uint32_t)((uint64_t)tp.tv_nsec);
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  time_sample->epoch_s = (uint32_t)tv.tv_sec;
  time_sample->offset_ns = (uint32_t)tv.tv_usec*1000LL;
#endif
  return ED247_STATUS_SUCCESS;
}

ed247_status_t libed247_update_time(
  ed247_time_sample_t time_sample,
  uint32_t epoch_s,
  uint32_t offset_ns)
{
  if(!time_sample) {
    PRINT_ERROR(__func__ << ": invalid time_sample pointer");
    return ED247_STATUS_FAILURE;
  }
  time_sample->epoch_s = epoch_s;
  time_sample->offset_ns = offset_ns;
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_load(
    const char *ecic_file_path,
    void* unused,
    ed247_context_t *context)
{
    return ed247_load_file(ecic_file_path, context);
}

ed247_status_t ed247_load_file(
    const char *ecic_file_path,
    ed247_context_t *context)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!context) {
    PRINT_ERROR(__func__ << ": Empty context pointer");
    return ED247_STATUS_FAILURE;
  }
  *context = nullptr;
  if(!ecic_file_path){
    PRINT_ERROR(__func__ << ": Empty file");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = ed247::Context::Builder::create_filepath(ecic_file_path);
    ed247_context->initialize();
    *context = ed247_context;
  }
  LIBED247_CATCH("Load");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_load_content(
  const char *ecic_file_content,
  ed247_context_t *context)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!context) {
    PRINT_ERROR(__func__ << ": Empty context pointer");
    return ED247_STATUS_FAILURE;
  }
  *context = nullptr;
  if(!ecic_file_content) {
    PRINT_ERROR(__func__ << ": Empty content");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = ed247::Context::Builder::create_content(ecic_file_content);
    ed247_context->initialize();
    *context = ed247_context;
  }
  LIBED247_CATCH("Load content");
  return ED247_STATUS_SUCCESS;
}

/***********
 * Finders *
 ***********/

ed247_status_t ed247_find_channels(
  ed247_context_t context,
  const char *regex_name,
  ed247_channel_list_t *channels)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!channels) {
    PRINT_ERROR(__func__ << ": Invalid channels pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    ed247::SmartListChannels * smart_channels = new ed247::SmartListChannels(ed247_context->getPoolChannels()->find(regex_name != nullptr ? std::string(regex_name) : std::string(".*")));
    smart_channels->set_managed(false);
    *channels = static_cast<ed247_channel_list_t>(smart_channels);
  }
  LIBED247_CATCH("Find channels");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_get_channel(
  ed247_context_t context,
  const char *name,
  ed247_channel_t *channel)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!name) {
    PRINT_ERROR(__func__ << ": Invalid name");
    return ED247_STATUS_FAILURE;
  }
  if(!channel) {
    PRINT_ERROR(__func__ << ": Invalid channel pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    auto && ed247_channel = ed247_context->getPoolChannels()->get(std::string(name));
    *channel = ed247_channel ? ed247_channel.get() : nullptr;
    if(*channel == nullptr) {
      PRINT_WARNING("Cannot finnd channel '" << name << "'");
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Get channel");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_find_streams(
  ed247_context_t context,
  const char *regex_name,
  ed247_stream_list_t *streams)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!streams) {
    PRINT_ERROR(__func__ << ": Invalid streams pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    ed247::SmartListStreams * smart_streams = new ed247::SmartListStreams(ed247_context->getPoolStreams()->find(regex_name != nullptr ? std::string(regex_name) : std::string(".*")));
    *streams = static_cast<ed247_stream_list_t>(smart_streams);
  }
  LIBED247_CATCH("Find streams");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_get_stream(
  ed247_context_t context,
  const char *name,
  ed247_stream_t *stream)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!name) {
    PRINT_ERROR(__func__ << ": Invalid name");
    return ED247_STATUS_FAILURE;
  }
  if(!stream) {
    PRINT_ERROR(__func__ << ": Invalid stream pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    auto && ed247_stream = ed247_context->getPoolStreams()->get(std::string(name));
    *stream = ed247_stream ? ed247_stream.get() : nullptr;
    if(*stream == nullptr) {
      PRINT_WARNING("Cannot find stream '" << name << "'");
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Get stream");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_find_channel_streams(
  ed247_channel_t channel,
  const char *regex_name,
  ed247_stream_list_t *streams)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!channel) {
    PRINT_ERROR(__func__ << ": Invalid channel");
    return ED247_STATUS_FAILURE;
  }
  if(!streams) {
    PRINT_ERROR(__func__ << ": Invalid streams pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_channel = (ed247::Channel*)(channel);
    ed247::SmartListStreams * smart_streams = new ed247::SmartListStreams(std::move(ed247_channel->find_streams(regex_name != nullptr ? std::string(regex_name) : std::string(".*"))));
    *streams = static_cast<ed247_stream_list_t>(smart_streams);
  }
  LIBED247_CATCH("Find channel streams");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_get_channel_stream(
  ed247_channel_t channel,
  const char *name,
  ed247_stream_t *stream)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!channel) {
    PRINT_ERROR(__func__ << ": Invalid channel");
    return ED247_STATUS_FAILURE;
  }
  if(!name) {
    PRINT_ERROR(__func__ << ": Invalid name");
    return ED247_STATUS_FAILURE;
  }
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_channel = (ed247::Channel*)(channel);
    auto && ed247_stream = ed247_channel->get_stream(std::string(name));
    *stream = ed247_stream ? ed247_stream.get() : nullptr;
    if(*stream == nullptr) {
      PRINT_WARNING("Cannot find channel '" << name << "'");
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Get channel stream");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_find_signals(
  ed247_context_t context,
  const char *regex_name,
  ed247_signal_list_t *signals)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!signals) {
    PRINT_ERROR(__func__ << ": Invalid signals pointer ");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    ed247::SmartListSignals * smart_signals = new ed247::SmartListSignals(ed247_context->getPoolSignals()->find(regex_name != nullptr ? std::string(regex_name) : std::string(".*")));
    smart_signals->reset();
    *signals = static_cast<ed247_signal_list_t>(smart_signals);
  }
  LIBED247_CATCH("Find signals");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_get_signal(
  ed247_context_t context,
  const char *name,
  ed247_signal_t *signal)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!name) {
    PRINT_ERROR(__func__ << ": Invalid name");
    return ED247_STATUS_FAILURE;
  }
  if(!signal) {
    PRINT_ERROR(__func__ << ": Invalid signal pointer ");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    auto && ed247_signal = ed247_context->getPoolSignals()->get(std::string(name));
    *signal = ed247_signal ? ed247_signal.get() : nullptr;
    if(*signal == nullptr) {
      PRINT_WARNING("Cannot find signal '" << name << "'");
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Get signal");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_find_stream_signals(
  ed247_stream_t stream,
  const char *regex_name,
  ed247_signal_list_t *signals)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!stream) {
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!signals){
    PRINT_ERROR(__func__ << ": Invalid signals pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_stream = (ed247::BaseStream*)(stream);
    ed247::SmartListSignals * smart_signals = new ed247::SmartListSignals(std::move(ed247_stream->find_signals(regex_name != nullptr ? std::string(regex_name) : std::string(".*"))));
    *signals = static_cast<ed247_signal_list_t>(smart_signals);
  }
  LIBED247_CATCH("Find stream signals");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_get_stream_signal(
  ed247_stream_t stream,
  const char *name,
  ed247_signal_t *signal)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!stream) {
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!name) {
    PRINT_ERROR(__func__ << ": Invalid name");
    return ED247_STATUS_FAILURE;
  }
  if(!signal) {
    PRINT_ERROR(__func__ << ": Invalid signal pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_stream = (ed247::BaseStream*)(stream);
    auto && ed247_signal = ed247_stream->get_signal(std::string(name));
    *signal = ed247_signal ? ed247_signal.get() : nullptr;
    if(*signal == nullptr) {
      PRINT_WARNING("Cannot find signal '" << name << "'");
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Get stream signal");
  return ED247_STATUS_SUCCESS;
}

/***********
 * Getters *
 ***********/

ed247_status_t ed247_component_get_info(
  ed247_context_t context,
  const ed247_component_info_t **info)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!info) {
    PRINT_ERROR(__func__ << ": Empty info pointer");
    return ED247_STATUS_FAILURE;
  }
  *info = nullptr;
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    *info = &ed247_context->getRoot()->info;
  }
  LIBED247_CATCH("Get component info");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_component_get_streams(
  ed247_context_t context,
  ed247_stream_list_t *streams)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!streams) {
    PRINT_ERROR(__func__ << ": Empty info pointer");
    return ED247_STATUS_FAILURE;
  }
  *streams = nullptr;
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    *streams = std::static_pointer_cast<ed247_internal_stream_list_t>(ed247_context->getPoolStreams()->streams()).get();
  }
  LIBED247_CATCH("Get streams info");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_component_get_channels(
  ed247_context_t context,
  ed247_channel_list_t *channels)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!channels) {
    PRINT_ERROR(__func__ << ": Empty info pointer");
    return ED247_STATUS_FAILURE;
  }
  *channels = nullptr;
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    auto schannels = std::static_pointer_cast<ed247_internal_channel_list_t>(ed247_context->getPoolChannels()->channels());
    *channels = schannels.get();
	
  }
  LIBED247_CATCH("Get channels info");
  return ED247_STATUS_SUCCESS;	
}

ed247_status_t ed247_component_set_user_data(
  ed247_context_t context,
  void *user_data)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    ed247_context->set_user_data(user_data);
  }
  LIBED247_CATCH("Set user data");
  return ED247_STATUS_SUCCESS;	
}

ed247_status_t ed247_component_get_user_data(
  ed247_context_t context,
  void **user_data)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!user_data) {
    PRINT_ERROR(__func__ << ": Empty user_data pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    *user_data = ed247_context->get_user_data();
  }
  LIBED247_CATCH("Get user data");
  return ED247_STATUS_SUCCESS;	
}

ed247_status_t ed247_channel_get_info(
  ed247_channel_t channel,
  const ed247_channel_info_t **info)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!channel) {
    PRINT_ERROR(__func__ << ": Invalid channel");
    return ED247_STATUS_FAILURE;
  }
  if(!info) {
    PRINT_ERROR(__func__ << ": Empty info pointer");
    return ED247_STATUS_FAILURE;
  }
  *info = nullptr;
  try{
    auto ed247_channel = static_cast<ed247::Channel*>(channel);
    *info = &ed247_channel->get_configuration()->info;
  }
  LIBED247_CATCH("Get channel info");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_channel_set_user_data(
  ed247_channel_t channel,
  void *user_data)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!channel) {
    PRINT_ERROR(__func__ << ": nvalid channel");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_channel = static_cast<ed247::Channel*>(channel);
    ed247_channel->set_user_data(user_data);
  }
  LIBED247_CATCH("Set channel user data");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_channel_get_user_data(
  ed247_channel_t channel,
  void **user_data)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!channel) {
    PRINT_ERROR(__func__ << ": Invalid channel");
    return ED247_STATUS_FAILURE;
  }
  if(!user_data) {
    PRINT_ERROR(__func__ << ": Invalid user_data");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_channel = static_cast<ed247::Channel*>(channel);
    ed247_channel->get_user_data(user_data);
  }
  LIBED247_CATCH("Get channel user data");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_channel_get_streams(
  ed247_channel_t channel,
  ed247_stream_list_t *streams)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!channel) {
    PRINT_ERROR(__func__ << ": Invalid channel");
    return ED247_STATUS_FAILURE;
  }
  if(!streams) {
    PRINT_ERROR(__func__ << ": Empty info pointer");
    return ED247_STATUS_FAILURE;
  }
  *streams = nullptr;
  try{
    auto ed247_channel = static_cast<ed247::Channel*>(channel);
    *streams = std::static_pointer_cast<ed247_internal_stream_list_t>(ed247_channel->sstreams()).get();
  }
  LIBED247_CATCH("Get streams info");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_get_info(
  ed247_stream_t stream,
  const ed247_stream_info_t **info)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!stream) {
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!info) {
    PRINT_ERROR(__func__ << ": Empty info pointer");
    return ED247_STATUS_FAILURE;
  }
  *info = nullptr;
  try{
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
    *info = &ed247_stream->get_configuration()->info;
  }
  LIBED247_CATCH("Get stream info");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_set_user_data(
  ed247_stream_t stream,
  void *user_data)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!stream) {
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
    ed247_stream->set_user_data(user_data);
  }
  LIBED247_CATCH("Set stream user data");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_get_user_data(
  ed247_stream_t stream,
  void **user_data)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!stream) {
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!user_data){
    PRINT_ERROR(__func__ << ": Invalid user_data");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
    ed247_stream->get_user_data(user_data);
  }
  LIBED247_CATCH("Get stream user data");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_get_channel(
  ed247_stream_t stream,
  ed247_channel_t *channel)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!channel){
    PRINT_ERROR(__func__ << ": Empty channel pointer");
    return ED247_STATUS_FAILURE;
  }
  *channel = nullptr;
  try{
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
    auto && ed247_channel = ed247_stream->get_channel();
    *channel = ed247_channel ? ed247_channel.get() : nullptr;
  }
  LIBED247_CATCH("Get stream channel");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_signal_get_info(
  ed247_signal_t signal,
  const ed247_signal_info_t **info)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!signal) {
    PRINT_ERROR(__func__ << ": Invalid signal");
    return ED247_STATUS_FAILURE;
  }
  if(!info) {
    PRINT_ERROR(__func__ << ": Empty info pointer");
    return ED247_STATUS_FAILURE;
  }
  *info = nullptr;
  try{
    auto ed247_signal = static_cast<ed247::BaseSignal*>(signal);
    *info = &ed247_signal->get_configuration()->info;
  }
  LIBED247_CATCH("Get signal info");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_signal_set_user_data(
  ed247_signal_t signal,
  void *user_data)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!signal){
    PRINT_ERROR(__func__ << ": Invalid signal");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_signal = static_cast<ed247::BaseSignal*>(signal);
    ed247_signal->set_user_data(user_data);
  }
  LIBED247_CATCH("Set signal user data");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_signal_get_user_data(
  ed247_signal_t signal,
  void **user_data)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!signal) {
    PRINT_ERROR(__func__ << ": Invalid signal");
    return ED247_STATUS_FAILURE;
  }
  if(!user_data){
    PRINT_ERROR(__func__ << ": Invalid user_data");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_signal = static_cast<ed247::BaseSignal*>(signal);
    ed247_signal->get_user_data(user_data);
  }
  LIBED247_CATCH("Get signal user data");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_signal_get_stream(
  ed247_signal_t signal,
  ed247_stream_t *stream)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!signal){
    PRINT_ERROR(__func__ << ": Invalid signal");
    return ED247_STATUS_FAILURE;
  }
  if(!stream){
    PRINT_ERROR(__func__ << ": Empty stream pointer");
    return ED247_STATUS_FAILURE;
  }
  *stream = nullptr;
  try{
    auto ed247_signal = static_cast<ed247::BaseSignal*>(signal);
    auto && ed247_stream = ed247_signal->get_stream();
    *stream = ed247_stream ? ed247_stream.get() : nullptr;
  }
  LIBED247_CATCH("Get signal stream");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_get_signals(
  ed247_stream_t stream,
  ed247_signal_list_t *signals)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!signals){
    PRINT_ERROR(__func__ << ": Empty signals pointer");
    return ED247_STATUS_FAILURE;
  }
  *signals = nullptr;
  try{
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
    auto ed247_signals = std::static_pointer_cast<ed247_internal_signal_list_t>(ed247_stream->signals());
    *signals = ed247_signals.get();
  }
  LIBED247_CATCH("Stream get signals");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_contains_signals(
  ed247_stream_t stream,
  uint8_t *yesno)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!yesno){
    PRINT_ERROR(__func__ << ": Empty yesno pointer");
    return ED247_STATUS_FAILURE;
  }
  *yesno = 0;
  try{
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
    *yesno = ed247_stream->signals()->size() != 0;
  }
  LIBED247_CATCH("Stream contains signals");
  return ED247_STATUS_SUCCESS;
}

/**********************************
 * Stream & Signal sample helpers *
 **********************************/

ed247_status_t ed247_stream_allocate_sample(
  ed247_stream_t stream,
  void ** sample_data,
  size_t * sample_size)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!sample_data){
    PRINT_ERROR(__func__ << ": Empty sample_data pointer");
    return ED247_STATUS_FAILURE;
  }
  if(!sample_size){
    PRINT_ERROR(__func__ << ": Empty sample_size pointer");
    return ED247_STATUS_FAILURE;
  }
  *sample_data = nullptr;
  *sample_size = 0;
  try{
    auto ed247_stream = (ed247::BaseStream*)(stream);
    auto sample = ed247_stream->allocate_sample();
    *sample_data = sample->data_rw();
    *sample_size = sample->capacity();
  }
  LIBED247_CATCH("Allocate stream sample");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_signal_allocate_sample(
  ed247_signal_t signal,
  void ** sample_data,
  size_t * sample_size)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!signal){
    PRINT_ERROR(__func__ << ": Invalid signal");
    return ED247_STATUS_FAILURE;
  }
  if(!sample_data){
    PRINT_ERROR(__func__ << ": Empty sample_data pointer");
    return ED247_STATUS_FAILURE;
  }
  if(!sample_size){
    PRINT_ERROR(__func__ << ": Empty sample_size pointer");
    return ED247_STATUS_FAILURE;
  }
  *sample_data = nullptr;
  *sample_size = 0;
  try{
    auto ed247_signal = (ed247::BaseSignal*)(signal);
    auto sample = ed247_signal->allocate_sample();
    *sample_data = sample->data_rw();
    *sample_size = sample->capacity();
  }
  LIBED247_CATCH("Allocate stream sample");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_get_assistant(
  ed247_stream_t stream,
  ed247_stream_assistant_t *assistant)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!assistant){
    PRINT_ERROR(__func__ << ": Empty assistant pointer");
    return ED247_STATUS_FAILURE;
  }
  *assistant = nullptr;
  try{
    auto ed247_stream = (ed247::BaseStream*)(stream);
    *assistant = ed247_stream->get_assistant().get();
    if(!*assistant || !ed247_stream->get_assistant()->is_valid()){
      PRINT_WARNING("Stream '" << ed247_stream->get_name() << "' do not have a valid stream signal assistant.");
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Get stream signal assistant");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_assistant_get_stream(
  ed247_stream_assistant_t assistant,
  ed247_stream_t *stream)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!assistant){
    PRINT_ERROR(__func__ << ": Invalid assistant");
    return ED247_STATUS_FAILURE;
  }
  if(!stream){
    PRINT_ERROR(__func__ << ": Empty stream pointer");
    return ED247_STATUS_FAILURE;
  }
  *stream = nullptr;
  try{
    auto ed247_assistant = static_cast<ed247::BaseStream::Assistant*>(assistant);
    *stream = ed247_assistant->get_stream().get();
  }
  LIBED247_CATCH("Get stream of assistant");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_assistant_write_signal(
  ed247_stream_assistant_t assistant,
  ed247_signal_t signal,
  const void *signal_sample_data,
  size_t signal_sample_size)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!assistant){
    PRINT_ERROR(__func__ << ": Invalid assistant");
    return ED247_STATUS_FAILURE;
  }
  if(!signal){
    PRINT_ERROR(__func__ << ": Invalid signal");
    return ED247_STATUS_FAILURE;
  }
  if(!signal_sample_data){
    PRINT_ERROR(__func__ << ": Empty signal sample data pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_assistant = static_cast<ed247::BaseStream::Assistant*>(assistant);
    auto && ed247_signal = static_cast<ed247::BaseSignal*>(signal)->shared_from_this();
    if (ed247_assistant->write(ed247_signal, signal_sample_data, signal_sample_size) == false) {
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Write signal sample in assistant");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_assistant_read_signal(
  ed247_stream_assistant_t assistant,
  ed247_signal_t signal,
  const void **signal_sample_data,
  size_t *signal_sample_size)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!assistant){
    PRINT_ERROR(__func__ << ": Invalid assistant");
    return ED247_STATUS_FAILURE;
  }
  if(!signal){
    PRINT_ERROR(__func__ << ": Invalid signal");
    return ED247_STATUS_FAILURE;
  }
  if(!signal_sample_data){
    PRINT_ERROR(__func__ << ": Empty signal sample data pointer");
    return ED247_STATUS_FAILURE;
  }
  if(!signal_sample_size){
    PRINT_ERROR(__func__ << ": Empty signal sample size pointer");
    return ED247_STATUS_FAILURE;
  }
  *signal_sample_data = nullptr;
  *signal_sample_size = 0;
  try{
    auto ed247_assistant = static_cast<ed247::BaseStream::Assistant*>(assistant);
    auto ed247_signal = static_cast<ed247::BaseSignal*>(signal)->shared_from_this();
    if (ed247_assistant->read(ed247_signal, signal_sample_data, signal_sample_size) == false) {
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Read signal sample from assistant");
  return ED247_STATUS_SUCCESS;
}

/************************
 * Read & write samples *
 ************************/

ed247_status_t ed247_stream_push_sample(
  ed247_stream_t stream,
  const void *sample_data,
  size_t sample_size,
  const ed247_timestamp_t *timestamp,
  bool *full)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!sample_data){
    PRINT_ERROR(__func__ << ": Invalid sample data");
    return ED247_STATUS_FAILURE;
  }
  try{
    if (static_cast<ed247::BaseStream*>(stream)->push_sample(sample_data, sample_size, timestamp, full) == false) {
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Push stream sample");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_push_samples(
  ed247_stream_t stream,
  const void *samples_data,
  const size_t *samples_size,
  size_t samples_number,
  const ed247_timestamp_t *timestamp,
  bool *full)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!samples_data){
    PRINT_ERROR(__func__ << ": Invalid sample data");
    return ED247_STATUS_FAILURE;
  }
  if(!samples_size){
    PRINT_ERROR(__func__ << ": Invalid sample size");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
    size_t sum_size = 0;
    for(uint32_t i = 0 ; i < samples_number ; i++){
      if (ed247_stream->push_sample((const char*)samples_data+sum_size, samples_size[i], timestamp, full) == false) {
        return ED247_STATUS_FAILURE;
      }
      sum_size += samples_size[i];
    }
  }
  LIBED247_CATCH("Push stream samples");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_assistant_push_sample(
  ed247_stream_assistant_t assistant,
  const ed247_timestamp_t *timestamp,
  bool *full)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!assistant){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_assistant = static_cast<ed247::BaseStream::Assistant*>(assistant);
    if (ed247_assistant->push(timestamp, full) == false) {
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Push stream sample with assistant");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_pop_sample(
  ed247_stream_t stream,
  const void **sample_data,
  size_t *sample_size,
  const ed247_timestamp_t **data_timestamp,
  const ed247_timestamp_t **recv_timestamp,
  const ed247_sample_info_t **info,
  bool *empty)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(sample_data == nullptr){
    PRINT_ERROR(__func__ << ": Empty sample_data pointer");
    return ED247_STATUS_FAILURE;
  }
  if(sample_size == nullptr){
    PRINT_ERROR(__func__ << ": Empty sample_size pointer");
    return ED247_STATUS_FAILURE;
  }
  *sample_data = nullptr;
  *sample_size = 0;
  try{
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
    if(ed247_stream->recv_stack().size() == 0) {
      PRINT_CRAZY("Stream '" << ed247_stream->get_name() << "': no data received.");
      return ED247_STATUS_NODATA;
    }
    auto sample = ed247_stream->pop_sample(empty);
    if (!sample) return ED247_STATUS_FAILURE;
    *sample_data = sample->data();
    *sample_size = sample->size();
    if(data_timestamp) *data_timestamp = sample->data_timestamp();
    if(recv_timestamp) *recv_timestamp = sample->recv_timestamp();
    if(info) *info = sample->info();
  }
  LIBED247_CATCH("Pop stream sample");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_assistant_pop_sample(
  ed247_stream_assistant_t assistant,
  const ed247_timestamp_t **data_timestamp,
  const ed247_timestamp_t **recv_timestamp,
  const ed247_sample_info_t **info,
  bool *empty)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!assistant){
    PRINT_ERROR(__func__ << ": Invalid assistant");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_assistant = static_cast<ed247::BaseStream::Assistant*>(assistant);
    if(ed247_assistant->get_stream()->recv_stack().size() == 0) {
      PRINT_CRAZY("Stream '" << ed247_assistant->get_stream()->get_name() << "': no data received.");
      return ED247_STATUS_NODATA;
    }
    if (ed247_assistant->pop(data_timestamp, recv_timestamp, info, empty) == false) {
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Pop stream sample with assistant");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_samples_number(
  ed247_stream_t stream,
  ed247_direction_t direction,
  size_t *size)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!size){
    PRINT_ERROR(__func__ << ": Empty size pointer");
    return ED247_STATUS_FAILURE;
  }
  if(direction != ED247_DIRECTION_IN && direction != ED247_DIRECTION_OUT){
    PRINT_ERROR(__func__ << ": Unknown direction");
    return ED247_STATUS_FAILURE;
  }
  *size = 0;
  try{
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
    *size = direction == ED247_DIRECTION_IN ? ed247_stream->recv_stack().size() : ed247_stream->send_stack().size();
  }
  LIBED247_CATCH("Stream samples number");
  return ED247_STATUS_SUCCESS;
}

/******************
 * Receive & Send *
 ******************/
ed247_status_t ed247_stream_register_recv_callback(
  ed247_context_t context,
  ed247_stream_t stream,
  ed247_stream_recv_callback_t callback)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!callback){
    PRINT_ERROR(__func__ << ": Invalid callback");
    return ED247_STATUS_FAILURE;
  }
  ed247_status_t status = ED247_STATUS_SUCCESS;
  try{
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
    if(ed247_stream->register_callback(context, callback) == ED247_STATUS_FAILURE){
      status = ED247_STATUS_FAILURE;
      PRINT_WARNING("Cannot register callback in stream [" << ed247_stream->get_name() << "]");
    }
  }
  LIBED247_CATCH("Stream register callback");
  return status;
}

ed247_status_t ed247_stream_unregister_recv_callback(
  ed247_context_t context,
  ed247_stream_t stream,
  ed247_stream_recv_callback_t callback)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!callback){
    PRINT_ERROR(__func__ << ": Invalid callback");
    return ED247_STATUS_FAILURE;
  }
  ed247_status_t status = ED247_STATUS_SUCCESS;
  try{
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
    if(ed247_stream->unregister_callback(context, callback) == ED247_STATUS_FAILURE){
      status = ED247_STATUS_FAILURE;
      PRINT_WARNING("Cannot unregister callback in stream [" << ed247_stream->get_name() << "]");
    }
  }
  LIBED247_CATCH("Stream unregister callback");
  return status;
}

ed247_status_t ed247_streams_register_recv_callback(
  ed247_context_t context,
  ed247_stream_list_t streams,
  ed247_stream_recv_callback_t callback)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!streams){
    PRINT_ERROR(__func__ << ": Invalid streams");
    return ED247_STATUS_FAILURE;
  }
  if(!callback){
    PRINT_ERROR(__func__ << ": Invalid callback");
    return ED247_STATUS_FAILURE;
  }
  ed247_status_t status = ED247_STATUS_SUCCESS;
  try{
    auto ed247_streams = static_cast<ed247::SmartListStreams*>(streams);
    ed247_streams->reset(); // Reset internal iterator
    std::shared_ptr<ed247::BaseStream> * next;
    while((next = ed247_streams->next_ok()) != nullptr){
      if((*next)->register_callback(context, callback) != ED247_STATUS_SUCCESS){
        status = ED247_STATUS_FAILURE;
        PRINT_WARNING("Cannot register callback in stream [" << (*next)->get_name() << "]");
      }
    }
  }
  LIBED247_CATCH("Streams register callback");
  return status;
}

ed247_status_t ed247_streams_unregister_recv_callback(
  ed247_context_t context,
  ed247_stream_list_t streams,
  ed247_stream_recv_callback_t callback)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!streams){
    PRINT_ERROR(__func__ << ": Invalid streams");
    return ED247_STATUS_FAILURE;
  }
  if(!callback){
    PRINT_ERROR(__func__ << ": Invalid callback");
    return ED247_STATUS_FAILURE;
  }
  ed247_status_t status = ED247_STATUS_SUCCESS;
  try{
    auto ed247_streams = static_cast<ed247::SmartListStreams*>(streams);
    ed247_streams->reset(); // Reset internal iterator
    std::shared_ptr<ed247::BaseStream> * next;
    while((next = ed247_streams->next_ok()) != nullptr){
      if((*next)->unregister_callback(context, callback) != ED247_STATUS_SUCCESS){
        status = ED247_STATUS_FAILURE;
        PRINT_WARNING("Cannot unregister callback in stream [" << (*next)->get_name() << "]");
      }
    }
  }
  LIBED247_CATCH("Streams unregister callback");
  return status;
}

ed247_status_t ed247_register_recv_callback(
  ed247_context_t context,
  ed247_stream_recv_callback_t callback)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!callback){
    PRINT_ERROR(__func__ << ": Invalid callback");
    return ED247_STATUS_FAILURE;
  }
  ed247_status_t status = ED247_STATUS_SUCCESS;
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    auto ed247_streams = ed247_context->getPoolStreams()->streams();
    ed247_streams->reset(); // Reset internal iterator
    std::shared_ptr<ed247::BaseStream> * next;
    while((next = ed247_streams->next_ok()) != nullptr){
      if((*next)->register_callback(context, callback) != ED247_STATUS_SUCCESS){
        status = ED247_STATUS_FAILURE;
        PRINT_WARNING("Cannot register callback in stream [" << (*next)->get_name() << "]");
      }
    }
  }
  LIBED247_CATCH("Register streams callback");
  return status;
}

ed247_status_t ed247_unregister_recv_callback(
  ed247_context_t context,
  ed247_stream_recv_callback_t callback)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!callback){
    PRINT_ERROR(__func__ << ": Invalid callback");
    return ED247_STATUS_FAILURE;
  }
  ed247_status_t status = ED247_STATUS_SUCCESS;
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    auto ed247_streams = ed247_context->getPoolStreams()->streams();
    ed247_streams->reset(); // Reset internal iterator
    std::shared_ptr<ed247::BaseStream> * next;
    while((next = ed247_streams->next_ok()) != nullptr){
      if((*next)->unregister_callback(context, callback) != ED247_STATUS_SUCCESS){
        status = ED247_STATUS_FAILURE;
        PRINT_WARNING("Cannot unregister callback in stream [" << (*next)->get_name() << "]");
      }
    }
  }
  LIBED247_CATCH("Unregister streams callback");
  return status;
}

extern LIBED247_EXPORT ed247_status_t ed247_register_com_recv_callback(
  ed247_context_t context,
  ed247_com_callback_t callback)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!callback){
    PRINT_ERROR(__func__ << ": Invalid callback");
    return ED247_STATUS_FAILURE;
  }
  ed247_status_t status = ED247_STATUS_FAILURE;
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    auto ed247_interfaces = std::static_pointer_cast<ed247::UdpSocket::Pool>(ed247_context->getPoolInterfaces());
    status = ed247_interfaces->register_recv_callback(callback, context);
  }
  LIBED247_CATCH("Register com recv callback");
  return status;
}

extern LIBED247_EXPORT ed247_status_t ed247_unregister_com_recv_callback(
  ed247_context_t context,
  ed247_com_callback_t callback)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!callback){
    PRINT_ERROR(__func__ << ": Invalid callback");
    return ED247_STATUS_FAILURE;
  }
  ed247_status_t status = ED247_STATUS_FAILURE;
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    auto ed247_interfaces = std::static_pointer_cast<ed247::UdpSocket::Pool>(ed247_context->getPoolInterfaces());
    status = ed247_interfaces->unregister_recv_callback(callback, context);
  }
  LIBED247_CATCH("Unregister com recv callback");
  return status;
}

extern LIBED247_EXPORT ed247_status_t ed247_register_com_send_callback(
  ed247_context_t context,
  ed247_com_callback_t callback)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!callback){
    PRINT_ERROR(__func__ << ": Invalid callback");
    return ED247_STATUS_FAILURE;
  }
  ed247_status_t status = ED247_STATUS_FAILURE;
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    auto ed247_interfaces = std::static_pointer_cast<ed247::UdpSocket::Pool>(ed247_context->getPoolInterfaces());
    status = ed247_interfaces->register_send_callback(callback, context);
  }
  LIBED247_CATCH("Register com send callback");
  return status;
}

extern LIBED247_EXPORT ed247_status_t ed247_unregister_com_send_callback(
  ed247_context_t context,
  ed247_com_callback_t callback)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!callback){
    PRINT_ERROR(__func__ << ": Invalid callback");
    return ED247_STATUS_FAILURE;
  }
  ed247_status_t status = ED247_STATUS_FAILURE;
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    auto ed247_interfaces = std::static_pointer_cast<ed247::UdpSocket::Pool>(ed247_context->getPoolInterfaces());
    status = ed247_interfaces->unregister_send_callback(callback, context);
  }
  LIBED247_CATCH("Unregister com send callback");
  return status;
}

ed247_status_t ed247_wait_frame(
  ed247_context_t context,
  ed247_stream_list_t *streams,
  int32_t timeout_us)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!streams){
    PRINT_ERROR(__func__ << ": Empty streams pointer");
    return ED247_STATUS_FAILURE;
  }
  *streams = nullptr;
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    auto ed247_status = ed247_context->wait_frame(timeout_us);
    if(ed247_status == ED247_STATUS_SUCCESS){
      *streams = ed247_context->active_streams().get();
    } else {
      PRINT_DEBUG("ed247_wait_frame status: " << ed247_status);
    }
    return ed247_status;
  }
  LIBED247_CATCH("ed247_wait_frame");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_wait_during(
  ed247_context_t context,
  ed247_stream_list_t *streams,
  int32_t duration_us)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!streams){
    PRINT_ERROR(__func__ << ": Empty streams pointer");
    return ED247_STATUS_FAILURE;
  }
  *streams = nullptr;
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    auto ed247_status = ed247_context->wait_during(duration_us);
    if(ed247_status == ED247_STATUS_SUCCESS){
      *streams = ed247_context->active_streams().get();
    } else {
      PRINT_DEBUG("ed247_wait_frame status: " << ed247_status);
    }
    return ed247_status;
  }
  LIBED247_CATCH("Wait during");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_send_pushed_samples(
  ed247_context_t context)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    ed247_context->send_pushed_samples();
  }
  LIBED247_CATCH("Send");
  return ED247_STATUS_SUCCESS;
}

// ADVANCED

ed247_status_t ed247_frame_encode(
  ed247_context_t context,
  ed247_frame_list_t *frames)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(frames == nullptr){
    PRINT_ERROR(__func__ << ": Empty frames pointer");
    return ED247_STATUS_FAILURE;
  }
  *frames = nullptr;
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    ed247_context->encode_frames();
    *frames = ed247_context->active_frames().get();
  }
  LIBED247_CATCH("Frame encode");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_frame_decode(
  ed247_channel_t channel,
  const void * data,
  size_t size)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!channel){
    PRINT_ERROR(__func__ << ": Invalid channel");
    return ED247_STATUS_FAILURE;
  }
  if(!data){
    PRINT_ERROR(__func__ << ": Empty data pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_channel = static_cast<ed247::Channel*>(channel);
    if (ed247_channel->decode((const char*)data, size) == false) {
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Frame decode");
  return ED247_STATUS_SUCCESS;
}

/**********
 * Unload *
 **********/

ed247_status_t ed247_unload(
  ed247_context_t context)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    delete(ed247_context);
    ed247_context=nullptr;
  }
  LIBED247_CATCH("Unload");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_free(
  void *data)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!data){
    PRINT_ERROR(__func__ << ": Invalid data");
    return ED247_STATUS_FAILURE;
  }
  try{
    free(data);
  }
  LIBED247_CATCH("Free");
  return ED247_STATUS_SUCCESS;
}
