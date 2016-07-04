#pragma once

template <typename T, size_t Alignment>
class aligned_allocator
{
public:
	typedef T * pointer;
	typedef const T * const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;
	typedef std::size_t size_type;
	typedef ptrdiff_t difference_type;

	aligned_allocator() {}
	aligned_allocator(const aligned_allocator&) {}
	template <typename U> aligned_allocator(const aligned_allocator<U, Alignment>&) {}
	~aligned_allocator() {}

	T* address(T& r) const { return &r; }
	const T* address(const T& s) const { return &s; }
	template <typename U> struct rebind { typedef aligned_allocator<U, Alignment> other; };
	bool operator!=(const aligned_allocator& other) const { return !(*this == other); }
	void construct(T* const p, const T& t) const { new (reinterpret_cast<void *>(p))T(t); }
	void destroy(T* const p) const { p->~T(); }
	bool operator==(const aligned_allocator& other) const { return true; }

	T* allocate(const std::size_t n) const
	{
		if (n == 0) {
			return NULL;
		}

		void* const pv = _mm_malloc(n * sizeof(T), Alignment);
		if (pv == NULL) {
			throw std::bad_alloc();
		}

		return reinterpret_cast<T *>(pv);
	}

	void deallocate(T * const p, const std::size_t n) const { _mm_free(p); }
	template <typename U> T* allocate(const std::size_t n, const U * /* const hint */) const { return allocate(n); }

private:
	aligned_allocator& operator=(const aligned_allocator&);
};

template <size_t Pagesize>
class static_allocator
{
public:
	char* allocate(const size_t n)
	{
		if (!m_pages.size() || m_used + n > Pagesize)
			allocate_page();

		auto ptr = m_pages.back()->data() + m_used;
		m_used += n;
		return ptr;
	}

	void deallocate_all()
	{
		for (page_t* page : m_pages)
			delete page;
		m_pages.clear();
	}

	size_t memoryUsed()
	{
		return (m_pages.size() - 1) * Pagesize + m_used;
	}

private:
	void allocate_page()
	{
		m_used = 0;
		m_pages.push_back(new page_t);
	}

	size_t m_used = 0;
	typedef std::array<char, Pagesize> page_t;
	std::vector<page_t *> m_pages;
};
