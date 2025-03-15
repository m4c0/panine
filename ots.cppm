export module ots;
import silog;
import vo;
import voo;

static constexpr const auto format = VK_FORMAT_R8G8B8A8_SRGB;

export class ots {
  voo::device_and_queue * m_dq;
  voo::single_cb m_cb { m_dq->queue_family() };
  voo::offscreen::buffers m_fb { m_dq->physical_device(), vo::extent, format };;

  int m_vframes {};

public:
  explicit ots(voo::device_and_queue * dq) : m_dq { dq } {}
  ~ots() {
    float time = m_vframes / 30.0;
    silog::log(silog::info, "Total frames in output: %d (%3.2fs)", m_vframes, time);
  }

  [[nodiscard]] constexpr auto render_pass() const { return m_fb.render_pass(); }
  [[nodiscard]] constexpr auto render_pass_begin() const { return m_fb.render_pass_begin({}); }

  void operator()(vo & v, auto && fn) {
    {
      voo::cmd_buf_one_time_submit ots { m_cb.cb() };
      fn(m_cb.cb());
      m_fb.cmd_copy_to_host(m_cb.cb());
    }
    m_dq->queue()->queue_submit({
      .command_buffer = m_cb.cb(),
    });
    vee::device_wait_idle();

    auto mm = m_fb.map_host();
    auto in = static_cast<vo::pix *>(*mm);
    auto out = v.lock();
    for (auto i = 0; i < vo::extent.width * vo::extent.height; i++) {
      *out = {{ in->p[3], in->p[0], in->p[1], in->p[2] }};
      out++;
      in++;
    }
    v.unlock(m_vframes++);
  }
};
