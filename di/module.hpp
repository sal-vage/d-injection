//          Copyright Adam Lach 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef DI_MODULE_HPP
#define DI_MODULE_HPP

#include <boost/type_traits/remove_pointer.hpp>
#include <di/detail/variadics.hpp>
#include <di/generic_builder.hpp>
#include <di/configuration.hpp>
#include <di/builder.hpp>

namespace di {
namespace detail {

template<bool control>
struct make_selective_use_call;

template<>
struct make_selective_use_call<false> {
	template<typename B, typename T> 
	void operator()(B&,const di::service<T>&) {
		//empty
	}
};

template<>
struct make_selective_use_call<true> {
	template<typename B, typename T>
	void operator()(B& builder, const di::service<T>& t) {
		builder.use(t);
	}
};

} // namespace detail

template<typename M>
struct module {
	typedef M module_type;

#ifdef DI_USE_BOOST_SHARED_PTR
	template<typename T>
	module<M>& use(const typename boost::shared_ptr<T>& element) {
		return use(di::service<T>(element));
	}
#else
	template<typename T>
	module<M>& use(const typename std::shared_ptr<T>& element) {
		return use(di::service<T>(element));
	}
#endif

	template<typename T>
	module<M>& use(const di::service<T>& element) {
		BOOST_MPL_ASSERT_MSG((boost::mpl::contains<typename M::provided::ref_type,di::service<T>*>::type::value), TypeIsNotOnModulesProvidedServiceList,);
		*boost::fusion::at_key<di::service<T>*>(provided) = element;
		return *this;
	}

	template<typename T>
	typename di::smart_ptr<T>::shared_ptr get() {
		BOOST_MPL_ASSERT_MSG((boost::mpl::contains<typename M::needed::ref_type,di::service<T>*>::type::value), TypeIsNotOnModulesNeededServiceList,);
		return *boost::fusion::at_key<di::service<T>*>(needed);
	}
	
	/**
	 * @brief Creates a builder preconfigured with applicable services.
	 */
	template<typename C>
	typename smart_ptr< di::builder<C> >::single_ptr builder() {
		di::builder<C>* abuilder = new di::builder<C>();
		boost::fusion::for_each(needed,builder_feeder< di::builder<C> >(*abuilder));
		return typename smart_ptr< di::builder<C> >::single_ptr(abuilder);
	}

	/**
	 * @brief Creates an abstract builder preconfigured with applicable services.
	 */
	template<typename C>
	typename smart_ptr< di::abstract_builder<C> >::single_ptr abstract_builder() {
		return this->abstract_builder<C,C>();
	}

	/**
	 * @brief Creates an abstract builder preconfigured with applicable services.
	 */
	template<typename I,typename C>
	typename smart_ptr< di::abstract_builder<I> >::single_ptr abstract_builder() {
		di::abstract_builder<I>* abuilder = new di::builder<C,I>();
		boost::fusion::for_each(needed,builder_feeder< di::abstract_builder<I> >(*abuilder));
		return typename smart_ptr< di::abstract_builder<I> >::single_ptr(abuilder);
	}

	/**
	 * @brief Creates a generic builder preconfigured with applicable services.
	 */
	template<typename C>
	typename smart_ptr< di::generic_builder<C> >::single_ptr generic_builder() {
		di::generic_builder<C>* abuilder = new di::generic_builder<C>();
		boost::fusion::for_each(needed,builder_feeder< di::generic_builder<C> >(*abuilder));
		return typename smart_ptr< di::generic_builder<C> >::single_ptr(abuilder);
	}

	/**
	 * @brief Implement this function in child module to be called when application::build() is invoked
	 */
	void build() {}

	/**
	 * @brief Implement this function in child module to be called when application::start() is invoked
	 */
	void start() {}

	/**
	 * @brief Implement this function in child module to be called when application::stop() is invoked
	 */
	void stop() {}

	/**
	 * @brief Implement this function in child module to be called when application::suspend() is invoked
	 */
	void suspend() {}

	/**
	 * @brief Implement this function in child module to be called when application::resume() is invoked
	 */
	void resume() {}

private:
	template<typename B>
	struct builder_feeder {
		builder_feeder(B& a_builder) : abuilder(&a_builder) {}
		template<typename T>
		void operator()(T& element) const {
			typedef typename boost::remove_pointer<T>::type::type raw_T;
			typedef typename boost::mpl::contains<typename B::subject::raw_type, raw_T>::type builders_subject_contains_T;
			
			detail::make_selective_use_call<builders_subject_contains_T::value>()(*abuilder,*element);
		}

		mutable B* abuilder;
	};

protected:
	typename M::provided::ref_type provided;
	typename M::needed::ref_type needed;
};

} //namspace di

#endif //DI_MODULE_HPP
