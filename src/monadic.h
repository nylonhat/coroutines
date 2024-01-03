#ifndef MONADIC_H
#define MONADIC_H

#include <functional>
#include "task.h"

template<typename F, typename... As>
auto fmap_impl(F f, As... as) -> Task<decltype(std::invoke(f, as.await_resume()...))>{
	co_return std::invoke(f, co_await as...);	
}

template<typename F, typename... As>
auto fmap(F&& f, As&&... as){
	return fmap_impl<F,As...>(std::forward<F>(f), std::forward<As>(as)...);
}

template<typename F, typename... As>
auto bind_impl(F f, As... as) -> decltype(std::invoke(f, as.await_resume()...)) {
	co_return co_await std::invoke(f, co_await as...);
}

template<typename F, typename... As>
auto bind(F&& f, As&&... as){
	return bind_impl<F, As...>(std::forward<F>(f), std::forward<As>(as)...);
}


#endif
