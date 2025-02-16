#pragma leco add_impl mov_impl
export module mov;
import silog;
import sitime;
import vee;
import voo;

extern "C" void * mov_alloc();
extern "C" void mov_dealloc(void *);
extern "C" void mov_frame(void *, double);
extern "C" unsigned * mov_data(void *, int *, int *);

export class mov : public voo::updater<voo::h2l_image> {
  sitime::stopwatch m_time;
  vee::physical_device m_pd;
  void * m_ptr;

  void update_data(voo::h2l_image * img) override {
    mov_frame(m_ptr, m_time.millis() / 1000.0);

    int w, h;
    auto ptr = mov_data(m_ptr, &w, &h);
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
  }

public:
  mov(vee::physical_device pd, voo::queue * q)
    : updater { q, {} }
    , m_pd { pd }
    , m_ptr { mov_alloc() } {}
  ~mov() { mov_dealloc(m_ptr); }

  auto image_view() const { return data().iv(); }
};
