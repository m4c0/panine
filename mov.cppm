#pragma leco add_impl mov_impl
export module mov;
import hai;
import jute;
import silog;
import sitime;
import vee;
import voo;

extern "C" {
  void * mov_alloc(const char * path, unsigned plen);
  void mov_dealloc(void *);
  int mov_begin_frame(void * m);
  int mov_frame_count(void * m);
  void mov_skip(void * m, int frames);
  unsigned * mov_frame(void * m, int * w, int * h);
  void mov_end_frame(void * m);
}

export class mov : public voo::updater<voo::h2l_image> {
  sitime::stopwatch m_time;
  vee::physical_device m_pd;
  hai::value<void *, mov_dealloc> m_ptr;
  unsigned m_frames {};

  void update_data(voo::h2l_image * img) override {
    mov_begin_frame(*m_ptr);

    int w, h;
    auto ptr = mov_frame(*m_ptr, &w, &h);
    if (img->width() != w || img->height() != h) {
      silog::log(silog::info, "new image with %dx%d", w, h);
      *img = voo::h2l_image {
        m_pd,
        static_cast<unsigned>(w), static_cast<unsigned>(h),
        VK_FORMAT_B8G8R8A8_SRGB
      };
    }

    voo::mapmem mm { img->host_memory() };
    auto mem = static_cast<unsigned *>(*mm);
    for (auto i = 0; i < w * h; i++) {
      *mem++ = *ptr++;
    }

    mov_end_frame(*m_ptr);
    m_frames++;
  }

public:
  mov(jute::view path, vee::physical_device pd, voo::queue * q)
    : updater { q, {} }
    , m_pd { pd }
    , m_ptr { mov_alloc(path.begin(), path.size()) }
  {
    silog::log(silog::info, "Loaded movie [%.*s]", static_cast<unsigned>(path.size()), path.begin());
  }
  ~mov() {
    float time = m_frames / 30.0;
    silog::log(silog::info, "Total frames played: %d (%3.2fs)", m_frames, time);
  }

  int length() { return mov_frame_count(*m_ptr); }
  void skip(int f) { mov_skip(*m_ptr, f); }

  auto image_view() const { return data().iv(); }
};
