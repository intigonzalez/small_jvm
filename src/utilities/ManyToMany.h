#ifndef __MANY_TO_MANY__
#define __MANY_TO_MANY__

#include <vector>

#include <forward_list>

template< typename T, typename H >
class TIterator;

template < typename T ,typename H>
class HIterator;

template< typename T, typename H >
class M2MRelationship {
	friend TIterator<T,H>;
	friend HIterator<T,H>;
private:
	std::forward_list< std::pair< T, H> > l;
	typedef typename std::forward_list< std::pair<T,H> >::iterator type_it;
public:

	typedef TIterator<T,H> iterator1;
	typedef HIterator<T,H> iterator2;

	void add(T t, H h) {
		std::pair<T,H> p = std::make_pair(t,h);
		type_it i = std::find(l.begin(), l.end(), p);
		if (i == l.end()) {
			l.push_front(p);
		}
	}

	void remove(T t, H h) {	
		l.remove_if([t,h](std::pair<T,H> p) {
			return p.first == t && p.second == h; 
		});
	}

	bool contains(T t, H h) {
		type_it i = std::find_if(l.begin(), l.end(), [t,h](std::pair<T,H> p) {
			return p.first == t && p.second == h;
		});
		return i != l.end();
	}

	bool contains2(H h) {
		type_it i = std::find_if(l.begin(), l.end(), [h](std::pair<T,H> p) {
			return p.second == h;
		});
		return i != l.end();
	}

	unsigned degree1(T t) {
		unsigned n = std::count_if(l.begin(), l.end(), [t](std::pair<T,H> p) {
			return p.first == t;
		});
		return n;
	}

	void removeAll1(T t) {
		l.remove_if([t](std::pair<T,H> p) {
			return p.first == t; 
		});
	}
	
	void removeAll2(H h) {
		l.remove_if([h](std::pair<T,H> p) {
			return p.second == h;
		});
	}

	iterator2 begin1(T t) {
		return iterator2(l.begin(), l.end(), t);
	}

	iterator2 end1(T t) {
		return iterator2(l.end(), l.end(), t);
	}

	iterator1 begin2(H h) {
		return iterator1(l.begin(), l.end(), h);
	}

	iterator1 end2(H h) {
		return iterator1(l.end(), l.end(), h);
	}
};

template< typename T, typename H >
class TIterator {
private:
	H h0;
	typename M2MRelationship<T, H>::type_it l;
	typename M2MRelationship<T, H>::type_it lEnd;
public:
	TIterator(typename M2MRelationship<T, H>::type_it it,
			typename M2MRelationship<T, H>::type_it itEnd,
			H h) : h0(h),l(it),lEnd(itEnd){
		while ( l != lEnd && (*l).second != h0)
			++l;
	}

	bool operator==(TIterator<T, H> other) {
		return l == other.l;
	}

	bool operator!=(TIterator<T, H> other) {
			return l != other.l;
	}

	TIterator& operator++() {
		if (l != lEnd) {
			++l;
			while ( l != lEnd && (*l).second != h0)
				++l;
		}
		return *this;
	}

	T& operator*() {
		return (*l).first;
	}

};

template< typename T, typename H >
class HIterator {
private:
	T t0;
	typename M2MRelationship<T, H>::type_it l;
	typename M2MRelationship<T, H>::type_it lEnd;
public:
	HIterator(typename M2MRelationship<T, H>::type_it it,
			typename M2MRelationship<T, H>::type_it itEnd,
			T t) : t0(t),l(it),lEnd(itEnd){
		while ( l != lEnd && (*l).first != t0)
			++l;
	}

	bool operator==(HIterator<T, H> other) {
		return l == other.l;
	}

	bool operator!=(HIterator<T, H> other) {
			return l != other.l;
	}

	HIterator& operator++() {
		if (l != lEnd) {
			++l;
			while ( l != lEnd && (*l).first != t0)
				++l;
		}
		return *this;
	}

	H& operator*() {
		return (*l).second;
	}
};


//static M2MRelationship<int, int> a;

#endif
