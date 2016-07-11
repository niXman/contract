
// Copyright Alexei Zakharov, 2013.
// Copyright niXman (i dot nixman dog gmail dot com) 2016.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef __contract_hpp__included
#define __contract_hpp__included

/***************************************************************************/

#include <cstddef>
#include <iostream>
#include <exception>
#include <functional>
#include <type_traits>

#define stringify__(x) stringify_imp__(x)
#define stringify_imp__(x) #x

// macros for variadic argument dispatch
#define arg_count__(...) arg_pos__(__VA_ARGS__, 5, 4, 3, 2, 1)
#define arg_pos__(_1,_2,_3,_4,_5, N, ...) N

#define concat__(macro, argc) concat2__(macro, argc)
#define concat2__(macro, argc) macro ## argc

/***************************************************************************/

#define CONTRACT_LIB_VERSION_MAJOR 0
#define CONTRACT_LIB_VERSION_MINOR 2
#define CONTRACT_LIB_VERSION_PATCH 3

#define CONTRACT_LIB_VERSION_STRING \
	stringify__(CONTRACT_LIB_VERSION_MAJOR) "." \
	stringify__(CONTRACT_LIB_VERSION_MINOR) "." \
	stringify__(CONTRACT_LIB_VERSION_PATCH)

#define CONTRACT_LIB_VERSION \
	CONTRACT_LIB_VERSION_MAJOR * 10000 + \
	CONTRACT_LIB_VERSION_MINOR * 100 + \
	CONTRACT_LIB_VERSION_PATCH

/***************************************************************************/

// interface: macros
//

// Define contract block.
//
// This macro defines a contract block for a specified `scope`.
//
// @scope  the scope of the contract:
//             `fun`     - defines a free function contract,
//             `mfun`    - defines a contract for a member-function,
//             `ctor`    - defines a contract for a constructor,
//             `dtor`    - defines a contract for a destructor,
//             `loop`    - defines a loop invariant contract,
//             `class`   - defines a contract for a class,
//             `derived` - defines a contract for a derived class.
#define CONTRACT(scope) contract_ ## scope ## __

// Define precondition contract.
//
// This macro defines a precondition check for a contract block defined by the
// `contract(...)` macro.  Precondition is checked in the following situations:
//   `fun`   - on function entry,
//   `mfun`  - on member-function entry,
//   `ctor`  - on constructor entry,
//   `dtor`  - on destructor entry,
//   `loop`  - not checked.
//
// @cond  precondition expression that should evalate to `true`.
// @msg   message which is reported to the contract violation handler if `cond`
//        evaluates to `false`.
//
// Use macro `CONTRACT_DISABLE_PRECONDITIONS` to disable precondition checking.
#define PRECONDITION(...) \
	concat__(PRECONDITION, arg_count__(__VA_ARGS__))(__VA_ARGS__)
#define PRECONDITION1(cond) PRECONDITION2(cond, #cond)

#if !defined(CONTRACT_DISABLE_PRECONDITIONS)
#	define PRECONDITION2(cond, msg) \
		contract_check__(precondition, cond, msg)
#else
#	define PRECONDITION2(cond, msg) \
		do {} while (false && (cond))
#endif

// Define postcondition contract.
//
// This macro defines a postcondition check for a contract block defined by the
// `contract(...)` macro.  Precondition is checked in the following situations:
//   `fun`   - on function exit,
//   `mfun`  - on member-function exit,
//   `ctor`  - on constructor exit,
//   `dtor`  - on destructor exit,
//   `loop`  - not checked.
//
// @cond  postcondition expression that should evalate to `true`.
// @msg   message which is reported to the contract violation handler if `cond`
//        evaluates to `false`.
//
// Use macro `CONTRACT_DISABLE_POSTCONDITIONS` to disable precondition checking.
#define POSTCONDITION(...) \
	concat__(POSTCONDITION, arg_count__(__VA_ARGS__))(__VA_ARGS__)
#define POSTCONDITION1(cond) POSTCONDITION2(cond, #cond)

#if !defined(CONTRACT_DISABLE_POSTCONDITIONS)
#	define POSTCONDITION2(cond, msg) \
		contract_check__(postcondition, cond, msg)
#else
#	define POSTCONDITION2(cond, msg) \
		do {} while (false && (cond))
#endif

// Define invariant contract.
//
// This macro defines an invariant check for a contract block defined by the
// `contract(...)` macro.  Invariant is checked in the following situations:
//   `fun`     - on function entry and exit,
//   `mfun`    - on member-function entry and exit,
//   `ctor`    - on constructor exit,
//   `dtor`    - on destructor entry,
//   `loop`    - on each loop iteration,
//   `class`   - on entry and exit of each method with a `contract(this) block,
//               on exit of constructors with a `contract(ctor)` block, unless
//                  an exception is thrown,
//               on entry to destructors with a `contract(dtor)` block,
//   `derived` - on entry and exit of each method with a `contract(this) block,
//               on exit of constructors with a `contract(ctor)` block unless
//                  an exception is thrown,
//               on entry to destructors with a `contract(dtor)` block.
//
// @cond  postcondition expression that should evalate to `true`.
// @msg   message which is reported to the contract violation handler if `cond`
//        evaluates to `false`.
//
// Use macro `CONTRACT_DISABLE_INVARIANTS` to disable precondition checking.
#define INVARIANT(...) \
	concat__(INVARIANT, arg_count__(__VA_ARGS__))(__VA_ARGS__)
#define INVARIANT1(cond) INVARIANT2(cond, #cond)

#if !defined(CONTRACT_DISABLE_INVARIANTS)
#	define INVARIANT2(cond, msg) \
		contract_check__(invariant, cond, msg)
#else
#	define INVARIANT2(cond, msg) \
		do {} while (false && (cond))
#endif

/***************************************************************************/

// implementation: macros
//

// Define contract for a free function.
#define contract_fun__ \
	auto contract_obj__ = contract::detail::contractor<void *>(0) \
	+ [&](contract::detail::contract_context const & contract_context__)

// Define contract for a member function.
#define contract_mfun__ \
	auto contract_obj__ = contract::detail::contractor< \
		std::remove_reference<decltype(*this)>::type \
	>(this) \
	+ [&](contract::detail::contract_context const & contract_context__)

// Define contract for a constructor.
#define contract_ctor__ \
	auto contract_obj__ = contract::detail::contractor< \
		std::remove_reference<decltype(*this)>::type \
	>(this, false, true) \
	+ [&](contract::detail::contract_context const & contract_context__)

// Define contract for a destructor.
#define contract_dtor__ \
	auto contract_obj__ = contract::detail::contractor< \
		std::remove_reference<decltype(*this)>::type \
	>(this, true, false) \
	+ [&](contract::detail::contract_context const & contract_context__)

// Define a class contract.
#define contract_class__ \
	template <typename T> \
	friend struct contract::detail::class_contract_base; \
	\
	template <typename T> \
	friend struct contract::detail::has_class_contract; \
	\
	template <typename ...Bases> \
	friend struct contract::detail::base_class_contract; \
	\
	contract::detail::contract_context prepare_contract__( \
		contract::detail::contract_context const & contract_context__) const \
	{ \
		return contract_context__; \
	} \
	\
	void class_contract__( \
		contract::detail::contract_context const & contract_context__) const

// Define a derived class contract.
#define contract_derived__(...) \
	template <typename T> \
	friend struct contract::detail::class_contract_base; \
	\
	template <typename T> \
	friend struct contract::detail::has_class_contract; \
	\
	template <typename ...Bases> \
	friend struct contract::detail::base_class_contract; \
	\
	contract::detail::contract_context prepare_contract__( \
		contract::detail::contract_context const & contract_context__) const \
	{ \
		contract::detail::base_class_contract<__VA_ARGS__>::enforce(this, contract_context__); \
		return contract_context__; \
	} \
	\
	void class_contract__( \
		contract::detail::contract_context const & contract_context__) const


// Define a loop invariant contract.
#define contract_loop__ \
	if (contract::detail::contract_context contract_context__{false, false, true})

// Contract check main implementation.
#define contract_check__(TYPE, COND, MSG) \
	do { \
		if (contract_context__.check_ ## TYPE() && !(COND)) \
			contract::handle_violation( \
				contract::violation_context( \
					contract::type::TYPE \
					,MSG \
					,#COND \
					,__FILE__ \
					,__LINE__ \
				) \
			); \
	} while (0)

/***************************************************************************/

namespace contract {

// interface: violation handler
//

// Values for types of contract checks.
//
// Enumeration that defines the values for types of contract checks.  These
// types correspond to the identically named contract check macros.  This
// enumeration is not usable directly.  Instead, contract check macros pass the
// appropriate enumeration value to the contractor of the <violation_context>
// class.
enum class type: std::uint8_t {
	 precondition
	,postcondition
	,invariant
};

// Context of the contract violation.
//
// Defines the context data passed to the <handle_violation> function when a
// contract check macro detects a contract violation.
struct violation_context {
	violation_context(contract::type t,
						char const * m,
						char const * c,
						char const * f,
						std::size_t l)
		: contract_type{t}
		, message{m}
		, condition{c}
		, file{f}
		, line{l}
	{}

	contract::type const contract_type; // type of the failed contract check macro
	char const * message;         // message passed to the contract check macro
	char const * condition;       // condition of the contract check
	char const * file;            // file in which the contract check occures
	std::size_t const line;             // line on which the contact check occures
};

// Handle contract violation.
//
// Handle the contract violation.  A contract is violated when a condition
// expression passed to a contract check macro evaluates to `false`.
//
// @context  the context data for the contract violation.
// @returns  this function doesn't return; it can either call another
//           `[[noreturn]]` function or exit via an exception.
[[noreturn]]
void handle_violation(violation_context const & context);

// Type alias for the contract violation handler function.
using violation_handler = std::function<void (violation_context const &)>;

// Set contract violation handler.
//
// Set the handler function which is invoked when a contract violation is
// detected by a contract check macro.
//
// @new_handler  new handler function.
// @returns      previous handler function.
violation_handler set_handler(violation_handler new_handler);

// Get current contract violation handler.
//
// Get the handler function which is invoked when a contract violation is
// detect by a contract check macro.
//
// @returns  current contract violation handler function.
violation_handler get_handler();

/***************************************************************************/

namespace detail {
	
// implementation: code behind macros
//

// Context in which a contract check is done.  Controls which parts of the
// contract are checked.
struct contract_context {
	contract_context(bool pre, bool post, bool inv)
		: check_pre{pre}
		, check_post{post}
		, check_inv{inv}
	{}

	explicit
	operator bool() { return true; }

	bool check_precondition()  const { return check_pre; }
	bool check_postcondition() const { return check_post && !std::uncaught_exception(); }
	bool check_invariant()     const { return check_inv; }

	const bool check_pre;
	const bool check_post;
	const bool check_inv;
};

// Performs the check for a function or method contract.  Parameterized with
// `ContrFunc` functor defining the actual contract in terms of <precondition>,
// <postcondition> and <invariant> macros.  Precondition is checked on function
// entry, postcondition is checked on function exit, and invariant is checked
// on both entry and exit unless specified otherwise.
template <typename ContrFunc>
struct fun_contract {
	explicit
	fun_contract(ContrFunc f, bool enter = true, bool exit = true)
		:contr_(f)
		,exit_{exit}
	{
		contr_(contract_context{true, false, enter});
	}

	~fun_contract() noexcept(false)
	{
		contr_(contract_context{false, true, exit_});
	}

	ContrFunc contr_;
	bool exit_;
};

// A base class that performs the check for a class contract.  Parameterized
// with `ContrFunc` functor defining the actual contract in terms of
// <precondition>, <postcondition> and <invariant> macros.  Precondition and
// postcondition are not checked.  Invariant is checked on entry and exit if
// specified.
template <typename T>
struct class_contract_base {
	class_contract_base(T const * obj, bool enter, bool exit)
		:obj_{obj}
		,exit_{exit}
	{
		if (enter)
			obj_->class_contract__(obj_->prepare_contract__(contract_context{false, false, true}));
	}

	~class_contract_base() noexcept(false)
	{
		if (exit_ && !std::uncaught_exception())
			obj_->class_contract__(obj_->prepare_contract__(contract_context{false, false, true}));
	}

	T const * obj_;
	bool exit_;
};

// Performs the check for a method and class contract.  Combines the
// functionality of <class_contract_base> and <fun_contract> classes.
template <typename T, typename ContrFunc>
struct class_contract
	:class_contract_base<T>
	,fun_contract<ContrFunc>
{
	class_contract(T const * obj, ContrFunc f, bool enter, bool exit)
		:class_contract_base<T>{obj, enter, exit}
		,fun_contract<ContrFunc>{f, enter, exit}
	{}
};

// Template metafunction that detects if a class has a class contract defined.
// Defines `type` as `std::true_type` if the class contract is detected and
// `std::false_type` otherwise.
template <typename T>
struct has_class_contract {
	template <typename U>
	static auto test(int) -> decltype(std::declval<U>().class_contract__(
											std::declval<contract_context>()),
										std::true_type{});
	template <typename U>
	static auto test(...) -> std::false_type;

	using type = decltype(test<T>(0));
};

// Enforces base class contracts for a derived class.
//
// `Bases`   - the list of base class types with class contracts that should be
//             enforced as part of the derived class contract.
// `Derived` - the class derived from each of the `Bases`.
template <typename ...Bases>
struct base_class_contract {
	template <typename Derived>
	static
	void enforce(Derived * obj, contract_context const & context) {}
};

template <typename Base, typename ...Bases>
struct base_class_contract<Base, Bases...> {
	template <typename T>
	static
	void do_enforce(T * obj, contract_context const & context,
					typename std::enable_if<has_class_contract<T>::type::value>::type * = 0)
	{
		obj->class_contract__(context);
	}

	template <typename T>
	static
	void do_enforce(T * obj, contract_context const & context,
					typename std::enable_if<! has_class_contract<T>::type::value>::type * = 0)
	{}

	template <typename Derived>
	static
	void enforce(Derived * obj, contract_context const & context)
	{
		do_enforce(static_cast<typename std::add_const<Base>::type *>(obj), context);
		base_class_contract<Bases...>::enforce(obj, context);
	}
};

// Defines a bootstrapper for a contract check implementation.  When combined
// with a `Func` functor defining the actual contract (by means of overloaded
// `operator+`) produces a concrete implementation for the contract check.
template <typename T, bool = has_class_contract<T>::type::value>
struct contractor;

// Specialization for a function contract or a method contract without a class
// contract.
template <typename T>
struct contractor<T, false> {
	explicit
	contractor(T const *, bool = true, bool = true) {}

	template <typename Func>
	fun_contract<Func> operator+(Func f) const
	{
		return fun_contract<Func>{f, true, true};
	}
};

// Specialization for a method contract with a class contract.
template <typename T>
struct contractor<T, true> {
	explicit
	contractor(T const * obj, bool enter = true, bool exit = true)
		: obj_{obj}
		, enter_{enter}
		, exit_{exit}
	{}

	template<typename Func>
	class_contract<T, Func> operator+(Func f) const
	{
		return class_contract<T, Func>{obj_, f, enter_, exit_};
	}

	T const * obj_;
	bool enter_;
	bool exit_;
};

// implementation: violation handler
//

// Defines a default contract violation handler.  Prints the information about
// the contract violation to the standard error and abort the program
// execution.
inline
void default_handler(violation_context const & context) {
	std::cerr << context.file << ':' << context.line
				<< ": error: contract violation of type '";

	char const * type_str;

	switch (context.contract_type) {
	case type::precondition:
		type_str = "precondition";
		break;
	case type::postcondition:
		type_str = "postcondition";
		break;
	case type::invariant:
		type_str = "invariant";
		break;
	}

	std::cerr << type_str << "'\n"
				<< "message:   " << context.message << "\n"
				<< "condition: " << context.condition << std::endl;

	std::terminate();
}

// Holder for the currently installed contract failure handler.
// Templated with a dummy type to be able to keep it in the header file.
template <typename = void>
struct handler_holder {
	static
	violation_handler current_handler;
};

template <typename T>
violation_handler handler_holder<T>::current_handler{default_handler};

} // namespace detail

/***************************************************************************/

inline
void handle_violation(violation_context const & context) {
	detail::handler_holder<>::current_handler(context);

	// if the handler returns, abort anyway to satisfy the [[noreturn]] contract
	std::terminate();
}

inline
violation_handler set_handler(violation_handler new_handler) {
	violation_handler old_handler = detail::handler_holder<>::current_handler;
	detail::handler_holder<>::current_handler = new_handler;
	return old_handler;
}

inline
violation_handler get_handler() {
	return detail::handler_holder<>::current_handler;
}

/***************************************************************************/
	
} // namespace contract

/***************************************************************************/

#endif // __contract_hpp__included
