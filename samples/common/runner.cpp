/*************************************************************************
 * Copyright (C) [2020] by Cambricon, Inc. All rights reserved
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *************************************************************************/

#include "runner.h"

#include <glog/logging.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

StreamRunner::StreamRunner(const std::string& data_path, const VideoDecoder::DecoderType decode_type, int dev_id)
    : decoder_(new VideoDecoder(this, decode_type, dev_id)), device_id_(dev_id), data_path_(data_path) {
  parser_.reset(new VideoParser(decoder_.get()));
  if (!parser_->Open(data_path.c_str())) {
    THROW_EXCEPTION(edk::Exception::INIT_FAILED, "[EasyDK Samples] [StreamRunner] Open video source failed");
  }

  // set mlu environment
  env_.SetDeviceId(dev_id);
  env_.BindDevice();
}

StreamRunner::~StreamRunner() {
  Stop();
  WaitForRunLoopExit();
}

void StreamRunner::DemuxLoop(const uint32_t repeat_time) {
  // set mlu environment
  env_.SetDeviceId(device_id_);
  env_.BindDevice();
  bool is_rtsp = parser_->IsRtsp();
  uint32_t loop_time = 0;

  try {
    while (Running()) {
      // frame rate control, 25 frame per second for local video
      int ret = parser_->ParseLoop(is_rtsp ? 0 : 40);
      if (ret == -1) {
        THROW_EXCEPTION(edk::Exception::UNAVAILABLE, "[EasyDK Samples] [StreamRunner] No video source");
      }

      if (ret == 1) {
        // eos
        if (repeat_time > loop_time++) {
          std::unique_lock<std::mutex> lk(eos_mut_);
          if (!eos_cond_.wait_for(lk, std::chrono::milliseconds(10000), [this] { return receive_eos_.load(); })) {
            LOG(WARNING) << "[EasyDK Samples] [StreamRunner] Wait Eos timeout in DemuxLoop ";
          }
          lk.unlock();
          parser_->Close();
          lk.lock();
          receive_eos_ = false;
          lk.unlock();
          if (!parser_->Open(data_path_.c_str())) {
            THROW_EXCEPTION(edk::Exception::INIT_FAILED, "[EasyDK Samples] [StreamRunner] Open video source failed");
          }
          LOG(INFO) << "[EasyDK Samples] [StreamRunner] Loop...";
          continue;
        } else {
          decoder_->SendEos();
          std::unique_lock<std::mutex> lk(eos_mut_);
          if (!eos_cond_.wait_for(lk, std::chrono::milliseconds(10000), [this] { return receive_eos_.load(); })) {
            LOG(WARNING) << "[EasyDK Samples] [StreamRunner] Wait Eos timeout in DemuxLoop";
          }
          LOG(INFO) << "[EasyDK Samples] [StreamRunner] End Of Stream";
          break;
        }
      }
    }
  } catch (edk::Exception& e) {
    LOG(ERROR) << "[EasyDK Samples] [StreamRunner] DemuxLoop failed. Error: " << e.what();
    Stop();
  }
  if (Running()) decoder_->SendEos();
  parser_->Close();
  Stop();
}

bool StreamRunner::RunLoop() {
  // set mlu environment
  env_.SetDeviceId(device_id_);
  env_.BindDevice();
  in_loop_.store(true);

  try {
    while (running_.load()) {
      // inference
      std::unique_lock<std::mutex> lk(mut_);

      if (!cond_.wait_for(lk, std::chrono::milliseconds(100), [this] { return !frames_.empty(); })) {
        continue;
      }
      edk::CnFrame frame = frames_.front();
      frames_.pop();
      lk.unlock();

      Process(std::move(frame));
    }
  } catch (edk::Exception& err) {
    LOG(ERROR) << "[EasyDK Samples] [StreamRunner] RunLoop failed. Error: " << err.what();
    running_.store(false);
    in_loop_.store(false);
    return false;
  }

  // uninitialize
  running_.store(false);
  in_loop_.store(false);
  return true;
}
