#pragma leco app
#pragma leco add_impl avspeech

import pipeline;
import silog;
import vee;
import vo;
import voo;

import jojo;
import jute;

// static voo::device_and_queue dq { "panine-render" };

// static constexpr const auto format = VK_FORMAT_R8G8B8A8_SRGB;
// static voo::offscreen::buffers fb { dq.physical_device(), vo::extent, format };
// static pipeline ppl { &dq, fb.render_pass(), false };
// 
// static voo::single_cb cb { dq.queue_family() };
// static vee::render_pass_begin rpb = fb.render_pass_begin({});
// 
// static vo v {};
// 
// static int i = 0;

static const auto script = jojo::read_cstr("out/script.txt");

extern "C" void speak(const char * txt, unsigned n);
extern "C" void speak_callback(float * data, unsigned samples) {
  silog::trace(samples);
}
extern "C" void speak_marker_callback(const char * str, unsigned offset) {
  silog::trace(jute::view::unsafe(str), offset);
}

int main() {
  speak(script.begin(), script.size());

  //ms.write([](float * data, unsigned count) {
  //  silog::trace(count);
  //});
  //  {
  //    voo::cmd_buf_one_time_submit ots { cb.cb() };
  //    ppl.run(cb.cb(), rpb, ms.current());
  //    fb.cmd_copy_to_host(cb.cb());
  //  }
  //  dq.queue()->queue_submit({
  //    .command_buffer = cb.cb(),
  //  });
  //  
  //  vee::device_wait_idle();

  //  {
  //    auto mm = fb.map_host();
  //    auto in = static_cast<vo::pix *>(*mm);
  //    auto out = v.lock();
  //    for (auto i = 0; i < vo::extent.width * vo::extent.height; i++) {
  //      *out = {{ in->p[3], in->p[0], in->p[1], in->p[2] }};
  //      out++;
  //      in++;
  //    }
  //    v.unlock(i++);
  //  }

  //  ppl.next_frame();
  //  vee::device_wait_idle();
  //while (ms.playing()) v.wait();
  //v.done();
  //while (!v.wait());
}
