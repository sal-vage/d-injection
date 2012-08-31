//          Copyright Adam Lach 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef DI_ASSEMBLER_HPP
#define DI_ASSEMBLER_HPP

#include <boost/mpl/for_each.hpp>
#include <di/service_list.hpp>
#include <di/module.hpp>

namespace di {
namespace detail {

struct wrap_in_module {
	template<typename T>
	struct apply {
		typedef di::module<T> type;
	};
	template<>
	struct apply<detail::void_> {
		typedef boost::mpl::empty_base type;
	};
};

//uses template recurrence to join all module::provided type sets
struct join_all {
	template<typename Seq>
	struct apply {
		typedef typename boost::mpl::begin<Seq>::type begin_iterator;
		typedef typename boost::mpl::erase<Seq,begin_iterator>::type sequence_without_first_element;
		typedef typename boost::mpl::joint_view<
			 typename begin_iterator::type::provided::type,
			 typename join_all::apply<sequence_without_first_element>::type>::type type;
	};
	template<>
	struct apply< boost::mpl::vector0<boost::mpl::na> > {
		typedef boost::mpl::vector0<boost::mpl::na> type;
	};
};

} // namespace detail

template <BOOST_PP_ENUM_BINARY_PARAMS(DI_MAX_NUM_INJECTIONS, typename M, =detail::void_ BOOST_PP_INTERCEPT)>
class assembler : public boost::mpl::inherit<BOOST_PP_ENUM_BINARY_PARAMS(DI_MAX_NUM_INJECTIONS, typename di::detail::wrap_in_module::apply<M, >::type BOOST_PP_INTERCEPT) >::type {
	typedef di::assembler<BOOST_PP_ENUM_BINARY_PARAMS(DI_MAX_NUM_INJECTIONS, typename M, BOOST_PP_INTERCEPT)> this_type;

public:
	assembler() {	
		boost::mpl::for_each<module_prototypes>(configure_modules(*this));
	}

private:
	struct configure_modules {
		explicit configure_modules(this_type& an_assembler) : configurator(an_assembler) {}
		template<typename M>
		void operator() (const M& param) {
			configurator.configure_intermodule_connections<M>(configurator.provided_by_modules);
		}
		this_type& configurator;
	};

	template<typename M, typename Seq>
	void configure_intermodule_connections(Seq& sequence) {
		boost::fusion::for_each(di::module<M>::provided,connect_provided<Seq>(sequence));
		boost::fusion::for_each(di::module<M>::needed,connect_provided<Seq>(sequence));
	}

	template<typename Seq>
	struct connect_provided {
		connect_provided(Seq& a_sequence) : sequence(a_sequence) {}
		template<typename T>
		void operator()(T*& element) const {
			BOOST_MPL_ASSERT_MSG((boost::mpl::contains<Seq,T>::type::value), NoModuleProvidesTheNeededService,);
			element = &boost::fusion::at_key<T>(sequence);
		}
		Seq& sequence;
	};


	typedef boost::mpl::vector<BOOST_PP_ENUM_BINARY_PARAMS(DI_MAX_NUM_INJECTIONS, typename M, BOOST_PP_INTERCEPT)> raw_module_prototypes;
	typedef typename detail::vector_without_voids<raw_module_prototypes>::type module_prototypes;

	typename boost::fusion::result_of::as_set<
		typename detail::join_all::apply<module_prototypes>::type
	>::type provided_by_modules;
};

} //namspace di

#endif //DI_ASSEMBLER_HPP
