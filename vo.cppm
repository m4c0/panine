#pragma leco add_impl video_out
export module vo;
import hai;
import vee;

struct pix { char p[4]; };

extern "C" {
  void * vo_new(int w, int h);
  void vo_delete(void *);
  void * vo_audio(void *);
  void vo_done(void *);
  pix * vo_lock(void * p);
  void vo_unlock(void * p, unsigned frame);
  bool vo_wait(void * p);
}

struct deleter {
  void operator()(void * p) { vo_delete(p); }
};

export class vo {
  hai::value_holder<void *, deleter> m_h {};

public:
  static constexpr const vee::extent extent { 1080, 1920 };
  using pix = ::pix;

  vo() : m_h { vo_new(extent.width, extent.height) } {}

  void * audio() const { return vo_audio(*m_h); }
  auto lock() const { return vo_lock(*m_h); }
  void unlock(int i) const { vo_unlock(*m_h, i); }
  void done() const { vo_done(*m_h); }

  bool wait() const { return vo_wait(*m_h); }
};
