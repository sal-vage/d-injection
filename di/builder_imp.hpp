//          Copyright Adam Lach 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef DI_BUILDER_IMP_HPP
#define DI_BUILDER_IMP_HPP

#include <boost/type_traits/is_base_of.hpp>
#include <di/detail/perform_injection.hpp>
#include <di/abstract_builder.hpp>
#include <di/diagnostics.hpp>

namespace di {

template<typename C, typename I = C>
class builder_imp : public di::abstract_builder<I>, di::detail::noncopyable {
	BOOST_MPL_ASSERT_MSG((boost::is_base_of<I,C>::value), FirstTemplateParameterDoesNotDeriveFromSecond,);

public:
	builder_imp() {}

	virtual I* build() {
		C* instance = new C;
		build_inject(instance);
		instance->constructed();
		return instance;
	}

	virtual void build(I& instance) {
		C* downcasted = static_cast<C*>(&instance);
		delegate_inject(downcasted);
		instance.constructed();
	}

private:
	virtual void out_of_bounds() {
		C::diagnostics::out_of_bounds();
	}

	void build_inject(C* instance) {
		bool succeeded = true;
		boost::fusion::for_each(di::configurable<I>::injections,
			di::detail::perform_injection<C>(instance,&succeeded));
		if(!succeeded) {
			C::diagnostics::build_unsatisfied_requirement(instance);
		}
	}

	void delegate_inject(C* instance) {
		bool succeeded = true;
		boost::fusion::for_each(di::configurable<I>::injections,
			di::detail::perform_injection<C>(instance,&succeeded));
		if(!succeeded) {
			C::diagnostics::delegate_unsatisfied_requirement(instance);
		}
	}
};

} //namspace di

#endif //DI_BUILDER_IMP_HPP

