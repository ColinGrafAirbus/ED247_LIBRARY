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

#ifndef _ED247_STREAM_H_
#define _ED247_STREAM_H_

#include "ed247_internals.h"
#include "ed247_xml.h"
#include "ed247_signal.h"

#include <memory>
#include <tuple>
#include <map>

namespace ed247
{

class Channel;
class BaseStream;
class BaseSignal;
class SmartListStreams;

class BaseSample
{
public:
  BaseSample():
    _data(nullptr),
    _size(0),
    _capacity(0)
    {}

  BaseSample(const BaseSample & other) = delete;
  BaseSample & operator = (const BaseSample & other) = delete;

  // Accessor
  const size_t& size() const     { return _size;      }
  const size_t& capacity() const { return _capacity;  }
  const char* data() const       { return _data;      }
  bool empty() const             { return _size == 0; }


  // Fill data & size. Return false if capacity() is too small.
  bool copy(const char* data, const size_t& size) {
    if (size > _capacity) {
      PRINT_DEBUG("ERROR: Cannot copy payload: internal buffer is too small (" << size << " > " << _capacity << ")");
      return false;
    }
    _size = size;
    memcpy(_data, data, _size);
    return true;
  }
  bool copy(const void* data, const size_t& size) { return copy((const char*)data, size); }

  void reset() { _size = 0; }

  // Direct access. You have to check capacity() yourself.
  char* data_rw()                     { return _data; }
  void set_size(const size_t & size)  { _size = size; }

  // Memort management
  void allocate(size_t capacity) {
    _capacity = capacity;
    if(_data != nullptr || _size != 0) THROW_ED247_ERROR("Sample already allocated");
    if(!_capacity) THROW_ED247_ERROR("Cannot allocate a sample with a capacity of 0");
    _data = (char*)malloc(_capacity);
    if(!_data) THROW_ED247_ERROR("Failed to allocate sample [" << _capacity <<"] !");
    memset(_data,0,_capacity);
    _size = 0;
  }
  bool allocated() const {
    return _data != nullptr;
  }
  void deallocate() {
    if(_data) free(_data);
    _data = nullptr;
  }

private:
  char*  _data;
  size_t _size;
  size_t _capacity;
};

class FrameHeader;
class StreamSample : public BaseSample
{
    public:
        explicit StreamSample():
            BaseSample(),
            _data_timestamp(LIBED247_TIMESTAMP_DEFAULT),
            _recv_timestamp(LIBED247_TIMESTAMP_DEFAULT),
            _info(LIBED247_SAMPLE_INFO_DEFAULT)
        {}

        StreamSample(const StreamSample & other) = delete;
        StreamSample & operator = (const StreamSample & other) = delete;

        using BaseSample::copy;  // Do not hide base class copy
        bool copy(const StreamSample & sample)
        {
          if (BaseSample::copy(sample.data(), sample.size()) == false) return false;
          set_data_timestamp(*sample.data_timestamp());
          set_recv_timestamp(*sample.recv_timestamp());
          set_info(*sample.info());
          return true;
        }

        void set_data_timestamp(const ed247_timestamp_t & data_timestamp)
        {
            _data_timestamp = data_timestamp;
        }

        const ed247_timestamp_t * data_timestamp() const
        {
            return &_data_timestamp;
        }

        void set_recv_timestamp(const ed247_timestamp_t & recv_timestamp)
        {
            _recv_timestamp = recv_timestamp;
        }

        const ed247_timestamp_t * recv_timestamp() const
        {
            return &_recv_timestamp;
        }

        void update_timestamp()
        {
            SimulationTimeHandler::get().update_timestamp(_recv_timestamp);
        }

        void set_info(const ed247_sample_info_t & info)
        {
            _info = info;
        }

        const ed247_sample_info_t * info() const
        {
            return &_info;
        }

        void update_info(const FrameHeader & header);

    protected:
        ed247_timestamp_t   _data_timestamp;
        ed247_timestamp_t   _recv_timestamp;
        ed247_sample_info_t _info;
};

class CircularStreamSampleBuffer {
    public:
        ~CircularStreamSampleBuffer()
        {
            for(auto sp_sample : _samples)
                if(sp_sample && sp_sample->allocated())
                    sp_sample->deallocate();
        }

        size_t size() const
        {
            return _index_size;
        }

        std::shared_ptr<StreamSample> & next_write()
        {
            return _samples[_index_write];
        }

        bool increment()
        {
            if(_index_size < _sample_max_number){
                _index_write = (_index_write+1) % _samples.size();
                update_size();
                return _index_size == _sample_max_number;
            }else{
                _index_write = (_index_write+1) % _samples.size();
                _index_read = (_index_read+1) % _samples.size();
                update_size();
                return true;
            }
        }

        std::shared_ptr<StreamSample> & pop_front(bool * empty = nullptr) // True if empty after pop
        {
            if(size() == 0){
                if(empty) *empty = true;
                return _samples[_index_read];
            }else{
                size_t index_read = _index_read;
                _index_read = (_index_read+1) % _samples.size();
                update_size();
                if(empty) *empty = (size() == 0);
                return _samples[index_read];
            }
        }

        std::shared_ptr<StreamSample> front()
        {
            return size() > 0 ? _samples[_index_read] : nullptr;
        }

        std::shared_ptr<StreamSample> at(size_t index)
        {
            return index < size() ? _samples[(_index_read+index)%_samples.size()] : nullptr;
        }

        std::shared_ptr<StreamSample> back()
        {
            return size() > 0 ? _samples[_index_write == 0 ? (_samples.size()-1) : (_index_write-1)] : nullptr;
        }

        void allocate(size_t sample_max_size_bytes, size_t sample_max_number)
        {
            _sample_max_size_bytes = sample_max_size_bytes;
            _sample_max_number = sample_max_number;
            _samples.resize(sample_max_number+1);
            for(auto iter = _samples.begin() ; iter != _samples.end() ; iter++){
                (*iter) = std::make_shared<StreamSample>();
                (*iter)->allocate(_sample_max_size_bytes);
            }
            _index_read = 0;
            _index_write = 0;
            update_size();
        }

        std::vector<std::shared_ptr<StreamSample>> & samples()
        {
            return _samples;
        }

        bool full()
        {
            return size() >= _sample_max_number;
        }

    protected:
        std::vector<std::shared_ptr<StreamSample>> _samples;
        size_t _index_read{0};
        size_t _index_write{0};
        size_t _sample_max_size_bytes{0};
        size_t _sample_max_number{0};
        size_t _index_size{0};

        void update_size()
        {
            _index_size = _index_write >= _index_read ? (_index_write - _index_read) : (_samples.size() + _index_write - _index_read);
        }
};

template<ed247_stream_type_t ... E>
struct StreamBuilder {
    StreamBuilder() {}
    std::shared_ptr<BaseStream> create(const ed247_stream_type_t & type, std::shared_ptr<xml::Stream> & configuration, std::shared_ptr<BaseSignal::Pool> & pool_signals);
};

template<ed247_stream_type_t T, ed247_stream_type_t ... E>
struct StreamBuilder<T, E...> : public StreamBuilder<E...>, private StreamTypeChecker<T> {
    StreamBuilder() : StreamBuilder<E...>() {}
    std::shared_ptr<BaseStream> create(const ed247_stream_type_t & type, std::shared_ptr<xml::Stream> & configuration, std::shared_ptr<BaseSignal::Pool> & pool_signals);
};

template<ed247_stream_type_t T>
struct StreamBuilder<T> : private StreamTypeChecker<T> {
    StreamBuilder() {}
    std::shared_ptr<BaseStream> create(const ed247_stream_type_t & type, std::shared_ptr<xml::Stream> & configuration, std::shared_ptr<BaseSignal::Pool> & pool_signals);
};

class FrameHeader;
class BaseStream : public ed247_internal_stream_t, public std::enable_shared_from_this<BaseStream>
{
    public:
        BaseStream():
            _user_data(NULL)
        {}
        BaseStream(std::shared_ptr<xml::Stream> & configuration):
            _configuration(configuration),
            _signals(std::make_shared<SmartListSignals>()),
            _user_data(NULL)
        {
            _signals->set_managed(true);
        }

        virtual ~BaseStream(){}

        void set_user_data(void *user_data)
        {
            _user_data = user_data;
        }

        void get_user_data(void **user_data)
        {
            *user_data = _user_data;
        }

        const xml::Stream * get_configuration() const
        {
            return _configuration.get();
        }

        std::string get_name() const
        {
            return _configuration ? std::string(_configuration->info.name) : std::string();
        }

        // Return false on failure
        bool push_sample(const void * sample_data, size_t sample_size, const ed247_timestamp_t * data_timestamp = nullptr, bool * full = nullptr);

        // Return an invalid shared_ptr on error (empty is not an error)
        std::shared_ptr<StreamSample> pop_sample(bool *empty = nullptr);

        CircularStreamSampleBuffer & recv_stack()
        {
            return _recv_stack;
        }

        CircularStreamSampleBuffer & send_stack()
        {
            return _send_stack;
        }

        virtual ed247_status_t check_sample_size(size_t sample_size) const = 0;
        virtual std::unique_ptr<StreamSample> allocate_sample() const = 0;

        virtual size_t encode(char * frame, size_t frame_size) = 0;

        // return false if the frame has not been successfully decoded
        virtual bool decode(const char * frame, size_t frame_size, const FrameHeader * header = nullptr) = 0;

        BaseSample & buffer()
        {
            return _buffer;
        }

        std::vector<std::shared_ptr<BaseSignal>> find_signals(std::string str_regex);

        std::shared_ptr<BaseSignal> get_signal(std::string str_name);

        std::shared_ptr<SmartListSignals> signals() { return _signals; };

        ed247_status_t register_callback(ed247_context_t context, ed247_stream_recv_callback_t callback)
        {
            auto it = std::find_if(_callbacks.begin(), _callbacks.end(),
                [&context, &callback](const std::pair<ed247_context_t,ed247_stream_recv_callback_t> & element){
                    return element.first == context && element.second == callback;
                });
            if(it != _callbacks.end()){
                return ED247_STATUS_FAILURE;
            }
            _callbacks.push_back(std::make_pair(context, callback));
            return ED247_STATUS_SUCCESS;
        }

        ed247_status_t unregister_callback(ed247_context_t context, ed247_stream_recv_callback_t callback)
        {
            auto it = std::find_if(_callbacks.begin(), _callbacks.end(),
                [&context, &callback](const std::pair<ed247_context_t,ed247_stream_recv_callback_t> & element){
                    return element.first == context && element.second == callback;
                });
            if(it == _callbacks.end()){
                return ED247_STATUS_FAILURE;
            }
            _callbacks.erase(it);
            return ED247_STATUS_SUCCESS;
        }

        std::shared_ptr<Channel> get_channel()
        {
            return _channel.lock();
        }

    protected:

        std::shared_ptr<xml::Stream> _configuration;
        std::weak_ptr<Channel> _channel;
        CircularStreamSampleBuffer _recv_stack;
        std::shared_ptr<StreamSample> _recv_working_sample; // Pointer on a recv_stack element
        CircularStreamSampleBuffer _send_stack;
        std::shared_ptr<StreamSample> _send_working_sample; // New element, not pointer on a member of send_stack
        BaseSample _buffer;
        StreamSample _working_sample;
        std::shared_ptr<SmartListSignals> _signals;
        std::vector<std::pair<ed247_context_t,ed247_stream_recv_callback_t>> _callbacks;

    private:

        ed247_timestamp_t _data_timestamp;
        void *_user_data;

    protected:

        // Create elements in circular buffers (send/recv) according to the bus type
        virtual void allocate_stacks() = 0;

        virtual void allocate_buffer() = 0;

        virtual void allocate_working_sample() = 0;

        void register_channel(Channel & channel, ed247_direction_t direction);

        bool run_callbacks()
        {
          for(auto & pcallback : _callbacks)
          {
            if(pcallback.second) {
              ed247_status_t status = (*pcallback.second)(pcallback.first, this);
              if (status != ED247_STATUS_SUCCESS) {
                PRINT_DEBUG("User callback fail with return code: " << status);
                return false;
              }
            } else {
              PRINT_WARNING("Invalid user callback!");
              return false;
            }
          }
          return true;
        }

        void encode_data_timestamp(const std::shared_ptr<StreamSample> & sample, char * frame, size_t frame_size, size_t & frame_index)
        {
            if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
                if(frame_index == 0){
                    // Datatimestamp
                    if((frame_index + sizeof(uint32_t) + sizeof(uint32_t)) > frame_size) {
                      THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
                    }
                    _data_timestamp = *sample->data_timestamp();
                    *(uint32_t*)(frame+frame_index) = htonl(sample->data_timestamp()->epoch_s);
                    frame_index += sizeof(uint32_t);
                    *(uint32_t*)(frame+frame_index) = htonl(sample->data_timestamp()->offset_ns);
                    frame_index += sizeof(uint32_t);
                }else if(_configuration->data_timestamp.enable_sample_offset == ED247_YESNO_YES){
                    // Precise Datatimestamp
                    if((frame_index + sizeof(uint32_t)) > frame_size) {
                      THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
                    }
                    *(uint32_t*)(frame+frame_index) =
                        htonl((uint32_t)(((int32_t)sample->data_timestamp()->epoch_s - (int32_t)_data_timestamp.epoch_s)*1000000000
                        + ((int32_t)sample->data_timestamp()->offset_ns - (int32_t)_data_timestamp.offset_ns)));
                    frame_index += sizeof(int32_t);
                }
            }
        }

        // return false if the frame has not been successfully decoded
        bool decode_data_timestamp(const char * frame, const size_t & frame_size, size_t & frame_index, ed247_timestamp_t & data_timestamp, ed247_timestamp_t & timestamp)
        {
            if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
                if(frame_index == 0){
                    // Data Timestamp
                    if((frame_size-frame_index) < sizeof(ed247_timestamp_t)) {
                      PRINT_ERROR("Stream '" << get_name() << "': Invalid received frame size: " << frame_size);
                      return false;
                    }
                    timestamp.epoch_s = ntohl(*(uint32_t*)(frame+frame_index));
                    frame_index += sizeof(uint32_t);
                    timestamp.offset_ns = ntohl(*(uint32_t*)(frame+frame_index));
                    frame_index += sizeof(uint32_t);
                    data_timestamp = timestamp;
                    _working_sample.set_data_timestamp(data_timestamp);
                }else if(_configuration->data_timestamp.enable_sample_offset == ED247_YESNO_YES){
                    // Precise Data Timestamp
                    int32_t offset_ns = (int32_t)ntohl(*(uint32_t*)(frame+frame_index));
                    frame_index += sizeof(uint32_t);
                    timestamp = data_timestamp;
                    timestamp.epoch_s += offset_ns / 1000000000;
                    timestamp.offset_ns += (offset_ns - (offset_ns / 1000000000) * 1000000000);
                    _working_sample.set_data_timestamp(timestamp);
                }else{
                    // No precise data timestamp, use the first one
                    _working_sample.set_data_timestamp(data_timestamp);
                }
            }
            return true;
        }

    public:
        class Pool : public std::enable_shared_from_this<Pool>
        {
            public:
                Pool();
                Pool(std::shared_ptr<BaseSignal::Pool> & pool_signals);

                ~Pool(){};

                std::shared_ptr<BaseStream> get(std::shared_ptr<xml::Stream> & configuration);

                std::vector<std::shared_ptr<BaseStream>> find(std::string str_regex);

                std::shared_ptr<BaseStream> get(std::string str_name);

                std::shared_ptr<SmartListStreams> streams();

                size_t size() const;

            private:
                std::shared_ptr<SmartListStreams> _streams;
                std::shared_ptr<BaseSignal::Pool> _pool_signals;
                StreamBuilder<
                    ED247_STREAM_TYPE_A429,
                    ED247_STREAM_TYPE_A664,
                    ED247_STREAM_TYPE_A825,
                    ED247_STREAM_TYPE_SERIAL,
                    ED247_STREAM_TYPE_ETHERNET,
                    ED247_STREAM_TYPE_AUDIO,
                    ED247_STREAM_TYPE_DISCRETE,
                    ED247_STREAM_TYPE_ANALOG,
                    ED247_STREAM_TYPE_NAD,
                    ED247_STREAM_TYPE_VNAD> _builder;

        };
        class Builder
        {
            public:
                void build(std::shared_ptr<Pool> & pool, std::shared_ptr<xml::Stream> & configuration, Channel & channel) const;
        };
        class Assistant : public ed247_internal_stream_assistant_t
        {
            public:

                Assistant() {}
                Assistant(std::shared_ptr<BaseStream> stream):
                    _stream(stream)
                {
                    size_t capacity = 0;
                    for(auto signal : *stream->_signals){
                        auto send_sample = signal->allocate_sample();
                        auto recv_sample = signal->allocate_sample();
                        capacity += send_sample->capacity();
                        if(signal->get_configuration()->info.type == ED247_SIGNAL_TYPE_VNAD)
                            capacity += sizeof(uint16_t);
                        if(_send_samples.size() <= signal->position())
                            _send_samples.resize(signal->position()+1);
                        _send_samples[signal->position()].first = signal;
                        _send_samples[signal->position()].second = std::move(send_sample);
                        if(_recv_samples.size() <= signal->position())
                            _recv_samples.resize(signal->position()+1);
                        _recv_samples[signal->position()].first = signal;
                        _recv_samples[signal->position()].second = std::move(recv_sample);
                    }
                    _buffer.allocate(capacity);
                }

                std::shared_ptr<BaseStream> get_stream()
                {
                    return _stream;
                }

                bool is_valid()
                {
                    auto type = _stream->get_configuration()->info.type;
                    return _stream ? (
                        type == ED247_STREAM_TYPE_DISCRETE ||
                        type == ED247_STREAM_TYPE_ANALOG ||
                        type == ED247_STREAM_TYPE_NAD ||
                        type == ED247_STREAM_TYPE_VNAD) : false;
                }

                bool write(std::shared_ptr<BaseSignal> signal, const void *data, size_t size)
                {
                  if(!(_stream->get_configuration()->info.direction & ED247_DIRECTION_OUT)) {
                    PRINT_ERROR("Stream '" << _stream->get_name() << "': Cannot write Signal [" << signal->get_name() << "] to an non-output stream");
                    return false;
                  }
                  if(signal->get_configuration()->info.type == ED247_SIGNAL_TYPE_VNAD){
                    size_t sample_max_size = signal->get_configuration()->info.info.vnad.max_length * (BaseSignal::sample_max_size_bytes(signal->get_configuration()->info) + sizeof(uint16_t));
                    if(size > sample_max_size) {
                      PRINT_ERROR("Stream '" << _stream->get_name() << "': Cannot write Signal [" << signal->get_name() << "] "
                                  "as Signal SampleMaxSizeBytes is [" << sample_max_size << "] and data to write is of size [" << size << "]");
                      return false;
                    }
                  }
                  auto & sample = _send_samples[signal->position()].second;
                  if (sample->copy(data, size) == false) {
                    PRINT_ERROR("Stream '" << _stream->get_name() << "': Cannot write Signal [" << signal->get_name() << "]: invalid size: " << size);
                    return false;
                  }
                  return true;
                }

                bool read(std::shared_ptr<BaseSignal> signal, const void **data, size_t * size)
                {
                  if(!(_stream->get_configuration()->info.direction & ED247_DIRECTION_IN)) {
                    PRINT_ERROR("Stream '" << _stream->get_name() << "': Cannot read Signal [" << signal->get_name() << "] from a non-input stream");
                    return false;
                  }
                  auto & sample = _recv_samples[signal->position()].second;
                  *data = sample->data();
                  *size = sample->size();
                  return true;
                }

                bool push(const ed247_timestamp_t * data_timestamp = nullptr, bool * full = nullptr)
                {
                  if(!(_stream->get_configuration()->info.direction & ED247_DIRECTION_OUT)) {
                    PRINT_ERROR("Stream '" << _stream->get_name() << "': Cannot push to a non-output stream");
                    return false;
                  }
                  encode();
                  return _stream->push_sample(_buffer.data(), _buffer.size(), data_timestamp, full);
                }

                // return false if the signal has not been successfully decoded
                bool pop(const ed247_timestamp_t **data_timestamp = nullptr, const ed247_timestamp_t **recv_timestamp = nullptr, const ed247_sample_info_t **info = nullptr, bool *empty = nullptr)
                {
                    if(!(_stream->get_configuration()->info.direction & ED247_DIRECTION_IN)) {
                        PRINT_ERROR("Stream '" << _stream->get_name() << "': Cannot pop from a non-input stream");
                        return false;
                    }
                    auto sample = _stream->pop_sample(empty);
                    if (!sample) return false;
                    if(data_timestamp)*data_timestamp = sample->data_timestamp();
                    if(recv_timestamp)*recv_timestamp = sample->recv_timestamp();
                    if(info)*info = sample->info();
                    return decode(sample->data(), sample->size());
                };

                const BaseSample & buffer() { return _buffer; }

           private:
                void encode()
                {
                    size_t buffer_index = 0;
                    bool vnad_behaviour = false;
                    for(auto & pair : _send_samples){
                        if(!pair.first)
                            continue;
                        if(pair.first->get_configuration()->info.type == ED247_SIGNAL_TYPE_VNAD){
                            *(uint16_t*)(_buffer.data_rw()+buffer_index) = (uint16_t)htons((uint16_t)pair.second->size());
                            buffer_index += sizeof(uint16_t);
                            vnad_behaviour = true;
                        }else{
                            if(pair.first->get_configuration()->info.type == ED247_SIGNAL_TYPE_ANALOG){
                                buffer_index = (size_t)pair.first->get_configuration()->info.info.ana.byte_offset;
                            }else if(pair.first->get_configuration()->info.type == ED247_SIGNAL_TYPE_DISCRETE){
                                buffer_index = (size_t)pair.first->get_configuration()->info.info.dis.byte_offset;
                            }else if(pair.first->get_configuration()->info.type == ED247_SIGNAL_TYPE_NAD){
                                buffer_index = (size_t)pair.first->get_configuration()->info.info.nad.byte_offset;
                            }else{
                                THROW_ED247_ERROR("Signal [" << pair.first->get_name() << "] has not a valid type");
                            }
                        }
                        memcpy(_buffer.data_rw()+buffer_index, pair.second->data(), pair.second->size());
                        if(vnad_behaviour)
                            buffer_index += pair.second->size();
                        pair.second->reset();
                    }
                    if(vnad_behaviour)
                        _buffer.set_size(buffer_index);
                    else
                        _buffer.set_size(_stream->get_configuration()->info.sample_max_size_bytes);
                }

                // return false if the data has not been successfully decoded
                bool decode(const void * data, size_t size)
                {
                    for(auto & pair : _recv_samples){
                        pair.second->reset();
                    }
                    size_t buffer_index = 0;
                    bool vnad_behaviour = false;
                    for(auto & pair : _recv_samples){
                        if(!pair.first)
                            continue;
                        size_t sample_size = 0;
                        if(pair.first->get_configuration()->info.type == ED247_SIGNAL_TYPE_VNAD){
                            if(buffer_index + sizeof(uint16_t) > size) {
                              PRINT_ERROR("Signal '" << pair.first->get_name() << "': invalid VNAD size : " << size);
                              return false;
                            }
                            sample_size = ntohs(*(uint16_t*)((const char*)data+buffer_index));
                            buffer_index += sizeof(uint16_t);
                            vnad_behaviour = true;
                        }else{
                            sample_size = BaseSignal::sample_max_size_bytes(pair.first->get_configuration()->info);
                            if(pair.first->get_configuration()->info.type == ED247_SIGNAL_TYPE_ANALOG){
                                buffer_index = (size_t)pair.first->get_configuration()->info.info.ana.byte_offset;
                            }else if(pair.first->get_configuration()->info.type == ED247_SIGNAL_TYPE_DISCRETE){
                                buffer_index = (size_t)pair.first->get_configuration()->info.info.dis.byte_offset;
                            }else if(pair.first->get_configuration()->info.type == ED247_SIGNAL_TYPE_NAD){
                                buffer_index = (size_t)pair.first->get_configuration()->info.info.nad.byte_offset;
                            }else{
                              PRINT_ERROR("Signal '" << pair.first->get_name() << "': Invalid type: " << pair.first->get_configuration()->info.type);
                              return false;
                            }
                        }
                        pair.second->copy((const char*)data+buffer_index, sample_size);
                        if(vnad_behaviour)
                            buffer_index += sample_size;
                    }
                    return true;
                }

                std::shared_ptr<BaseStream> _stream;
                std::vector<std::pair<std::shared_ptr<BaseSignal>, std::unique_ptr<BaseSample>>> _send_samples;
                std::vector<std::pair<std::shared_ptr<BaseSignal>, std::unique_ptr<BaseSample>>> _recv_samples;
                BaseSample _buffer;

                FRIEND_TEST(SignalContext, SinglePushPop);
        };

    protected:
        std::shared_ptr<Assistant> _assistant;

    public:
        std::shared_ptr<Assistant> get_assistant() { return _assistant; }

};

template<ed247_stream_type_t E>
class Stream : public BaseStream, private StreamTypeChecker<E>
{
    public:
        const ed247_stream_type_t type {E};

        using BaseStream::BaseStream;

        virtual size_t encode(char * frame, size_t frame_size) override final;

        // return false if the frame has not been successfully decoded
        virtual bool decode(const char * frame, size_t frame_size, const FrameHeader * header = nullptr) final;

        virtual ed247_status_t check_sample_size(size_t sample_size) const;

        virtual std::unique_ptr<StreamSample> allocate_sample() const final
        {
            return std::move(allocate_sample_impl());
        }

    protected:
        virtual void allocate_stacks() final;
        virtual void allocate_buffer() final;
        virtual void allocate_working_sample() final;

        template<ed247_stream_type_t T = E>
        typename std::enable_if<!StreamSignalTypeChecker<T>::value,std::unique_ptr<StreamSample>>::type
        allocate_sample_impl() const;

        template<ed247_stream_type_t T = E>
        typename std::enable_if<StreamSignalTypeChecker<T>::value,std::unique_ptr<StreamSample>>::type
        allocate_sample_impl() const;

    public:
        class Builder
        {
            public:
                template<ed247_stream_type_t T = E>
                typename std::enable_if<!StreamSignalTypeChecker<T>::value, std::shared_ptr<Stream<E>>>::type
                create(std::shared_ptr<xml::Stream> & configuration,
                    std::shared_ptr<BaseSignal::Pool> & pool_signals) const;

                template<ed247_stream_type_t T = E>
                typename std::enable_if<StreamSignalTypeChecker<T>::value, std::shared_ptr<Stream<E>>>::type
                create(std::shared_ptr<xml::Stream> & configuration,
                    std::shared_ptr<BaseSignal::Pool> & pool_signals) const;
        };
};

class SmartListStreams : public SmartList<std::shared_ptr<BaseStream>>, public ed247_internal_stream_list_t
{
    public:
        using SmartList<std::shared_ptr<BaseStream>>::SmartList;
        virtual ~SmartListStreams() {}
};

class SmartListActiveStreams : public SmartListStreams
{
    public:
        using SmartListStreams::SmartListStreams;
        virtual ~SmartListActiveStreams() {}

        virtual std::shared_ptr<BaseStream> * next_ok() override
        {
            next_iter();
            _iter = std::find_if(_iter, std::vector<std::shared_ptr<BaseStream>>::end(),
                [](const std::shared_ptr<BaseStream> & sp){ return sp->recv_stack().size() > 0; });
            return current_value();
        }
};

};

#endif
