#pragma leco add_impl mov_impl
export module mov;

extern "C" void * mov_alloc();
extern "C" void mov_dealloc(void *);
extern "C" void mov_frame(void *, double);
extern "C" void * mov_data(void *, int *, int *);

export class mov {
  void * m_ptr;
public:
  mov() : m_ptr { mov_alloc() } {}
  ~mov() { mov_dealloc(m_ptr); }

  void frame(int ms) { mov_frame(m_ptr, ms / 1000.0); }
  void * data(int * w, int * h) { return mov_data(m_ptr, w, h); }
};
