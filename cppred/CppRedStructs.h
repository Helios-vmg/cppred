#pragma once
#include "CommonTypes.h"
#include "CppRedConstants.h"
#include <limits>
#include <type_traits>
#include <algorithm>
#include <string>
#include <functional>

#define WRAPPER_CONST 

template <typename WrappedT, size_t Size, typename reading_f, typename writing_f>
class basic_IntegerWrapper{
public:
	struct callback_struct{
		reading_f r;
		writing_f w;
	};
	typedef WrappedT type;
	static const size_t size = Size;
protected:
	void *memory;
	callback_struct callbacks;

	void copy(const basic_IntegerWrapper &other){
		this->memory = other.memory;
		this->callbacks = other.callbacks;
	}
public:
	basic_IntegerWrapper(void *memory, const callback_struct &callbacks): memory(memory), callbacks(callbacks){}
	basic_IntegerWrapper(basic_IntegerWrapper &&other){
		this->copy(other);
	}
	basic_IntegerWrapper(const basic_IntegerWrapper &other){
		this->copy(other);
	}
	WrappedT operator=(const basic_IntegerWrapper &other) WRAPPER_CONST{
		auto ret = (WrappedT)other;
		*this = ret;
		return ret;
	}
	const basic_IntegerWrapper &operator=(basic_IntegerWrapper &&other) = delete;
	WrappedT operator=(WrappedT input) WRAPPER_CONST{
		this->callbacks.w(input, this->memory);
		return input;
	}
#define IntegerWrapper_UNARY_OVERLOAD(x) \
	WrappedT operator x() const{ \
		return x (WrappedT)*this; \
	}
	IntegerWrapper_UNARY_OVERLOAD(!)
	IntegerWrapper_UNARY_OVERLOAD(~)
	IntegerWrapper_UNARY_OVERLOAD(-)
	IntegerWrapper_UNARY_OVERLOAD(+)
#define IntegerWrapper_CREMENT_OVERLOAD(x) \
	WrappedT operator x() WRAPPER_CONST{ \
		auto temp = (WrappedT)*this; \
		return *this = x temp; \
	} \
	WrappedT operator x(int) WRAPPER_CONST{ \
		auto ret = *this; \
		x *this; \
		return ret; \
	}
	IntegerWrapper_CREMENT_OVERLOAD(++)
	IntegerWrapper_CREMENT_OVERLOAD(--)
#define IntegerWrapper_BINARY_OVERLOAD(x) \
	WrappedT operator x(WrappedT input) const{ \
		return (WrappedT)*this x input; \
	}
#if 1
	operator WrappedT() const{
		WrappedT ret;
		this->callbacks.r(ret, this->memory);
		return ret;
	}
	WrappedT value() const{
		return (WrappedT)*this;
	}
#else
	IntegerWrapper_BINARY_OVERLOAD(+)
	IntegerWrapper_BINARY_OVERLOAD(-)
	IntegerWrapper_BINARY_OVERLOAD(*)
	IntegerWrapper_BINARY_OVERLOAD(/)
	IntegerWrapper_BINARY_OVERLOAD(%)
	IntegerWrapper_BINARY_OVERLOAD(&)
	IntegerWrapper_BINARY_OVERLOAD(&&)
	IntegerWrapper_BINARY_OVERLOAD(|)
	IntegerWrapper_BINARY_OVERLOAD(||)
	IntegerWrapper_BINARY_OVERLOAD(^)
	IntegerWrapper_BINARY_OVERLOAD(==)
	IntegerWrapper_BINARY_OVERLOAD(!=)
	IntegerWrapper_BINARY_OVERLOAD(<)
	IntegerWrapper_BINARY_OVERLOAD(<=)
	IntegerWrapper_BINARY_OVERLOAD(>=)
	IntegerWrapper_BINARY_OVERLOAD(>)
	IntegerWrapper_BINARY_OVERLOAD(>>)
	IntegerWrapper_BINARY_OVERLOAD(<<)
#endif

#define IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(x) \
	WrappedT operator x(WrappedT input) WRAPPER_CONST{ \
		auto temp = (WrappedT)*this; \
		temp x input; \
		return *this = temp; \
	}
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(+=)
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(-=)
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(*=)
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(/=)
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(%=)
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(&=)
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(|=)
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(^=)
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(<<=)
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(>>=)
};

#if 1
template <typename T>
using IntegerWrapper_reading_f = void (*)(T &, const void *);
template <typename T>
using IntegerWrapper_writing_f = void (*)(const T &, void *);

template <typename T>
using ArbitraryIntegerWrapper_reading_f = std::function<void(T &, const void *)>;
template <typename T>
using ArbitraryIntegerWrapper_writing_f = std::function<void(const T &, void *)>;

template <typename WrappedT, size_t Size>
class IntegerWrapper : public basic_IntegerWrapper<WrappedT, Size, IntegerWrapper_reading_f<WrappedT>, IntegerWrapper_writing_f<WrappedT>>{
public:
	IntegerWrapper(void *memory, const callback_struct &callbacks): basic_IntegerWrapper(memory, callbacks){}
	IntegerWrapper(IntegerWrapper &&other): basic_IntegerWrapper(std::move(other)){}
	IntegerWrapper(const IntegerWrapper &other): basic_IntegerWrapper(other){}
	WrappedT operator=(const IntegerWrapper &other) WRAPPER_CONST{
		return basic_IntegerWrapper::operator=(other);
	}
	const IntegerWrapper &operator=(IntegerWrapper &&other){
		*this = (WrappedT)other;
		return *this;
	}
	WrappedT operator=(WrappedT input) WRAPPER_CONST{
		return basic_IntegerWrapper::operator=(input);
	}
};

template <typename WrappedT, size_t Size>
class ArbitraryIntegerWrapper : public basic_IntegerWrapper<WrappedT, Size, ArbitraryIntegerWrapper_reading_f<WrappedT>, ArbitraryIntegerWrapper_writing_f<WrappedT>>{
public:
	ArbitraryIntegerWrapper(void *memory, const callback_struct &callbacks): basic_IntegerWrapper(memory, callbacks){}
	ArbitraryIntegerWrapper(ArbitraryIntegerWrapper &&other): basic_IntegerWrapper(std::move(other)){}
	ArbitraryIntegerWrapper(const ArbitraryIntegerWrapper &other): basic_IntegerWrapper(other){}
	WrappedT operator=(const ArbitraryIntegerWrapper &other) WRAPPER_CONST{
		return basic_IntegerWrapper::operator=(other);
	}
	const ArbitraryIntegerWrapper &operator=(ArbitraryIntegerWrapper &&other){
		*this = (WrappedT)other;
		return *this;
	}
	WrappedT operator=(WrappedT input) WRAPPER_CONST{
		return basic_IntegerWrapper::operator=(input);
	}
};

#else
template <typename WrappedT, size_t Size>
class IntegerWrapper{
public:
	typedef void (*reading_f)(WrappedT &, const void *);
	typedef void (*writing_f)(const WrappedT &, void *);
	struct callback_struct{
		reading_f r;
		writing_f w;
	};
	typedef WrappedT type;
	static const size_t size = Size;
private:
	void *memory;
	callback_struct callbacks;

	void copy(const IntegerWrapper &other){
		this->memory = other.memory;
		this->callbacks = other.callbacks;
	}
public:
	IntegerWrapper(void *memory, const callback_struct &callbacks): memory(memory), callbacks(callbacks){}
	IntegerWrapper(IntegerWrapper &&other){
		this->copy(other);
	}
	IntegerWrapper(const IntegerWrapper &other){
		this->copy(other);
	}
	WrappedT operator=(const IntegerWrapper &other) const{
		auto ret = (WrappedT)other;
		*this = ret;
		return ret;
	}
	const IntegerWrapper &operator=(IntegerWrapper &&other) = delete;
	WrappedT operator=(WrappedT input) const{
		this->callbacks.w(input, this->memory);
		return input;
	}
#define IntegerWrapper_UNARY_OVERLOAD(x) \
	WrappedT operator x() const{ \
		return x (WrappedT)*this; \
	}
	IntegerWrapper_UNARY_OVERLOAD(!)
	IntegerWrapper_UNARY_OVERLOAD(~)
	IntegerWrapper_UNARY_OVERLOAD(-)
	IntegerWrapper_UNARY_OVERLOAD(+)
#define IntegerWrapper_CREMENT_OVERLOAD(x) \
	WrappedT operator x() const{ \
		auto temp = (WrappedT)*this; \
		return *this = x temp; \
	} \
	WrappedT operator x(int) const{ \
		auto ret = *this; \
		x *this; \
		return ret; \
	}
	IntegerWrapper_CREMENT_OVERLOAD(++)
	IntegerWrapper_CREMENT_OVERLOAD(--)
#define IntegerWrapper_BINARY_OVERLOAD(x) \
	WrappedT operator x(WrappedT input) const{ \
		return (WrappedT)*this x input; \
	}
#if 1
	operator WrappedT() const{
		WrappedT ret;
		this->callbacks.r(ret, this->memory);
		return ret;
	}
#else
	IntegerWrapper_BINARY_OVERLOAD(+)
	IntegerWrapper_BINARY_OVERLOAD(-)
	IntegerWrapper_BINARY_OVERLOAD(*)
	IntegerWrapper_BINARY_OVERLOAD(/)
	IntegerWrapper_BINARY_OVERLOAD(%)
	IntegerWrapper_BINARY_OVERLOAD(&)
	IntegerWrapper_BINARY_OVERLOAD(&&)
	IntegerWrapper_BINARY_OVERLOAD(|)
	IntegerWrapper_BINARY_OVERLOAD(||)
	IntegerWrapper_BINARY_OVERLOAD(^)
	IntegerWrapper_BINARY_OVERLOAD(==)
	IntegerWrapper_BINARY_OVERLOAD(!=)
	IntegerWrapper_BINARY_OVERLOAD(<)
	IntegerWrapper_BINARY_OVERLOAD(<=)
	IntegerWrapper_BINARY_OVERLOAD(>=)
	IntegerWrapper_BINARY_OVERLOAD(>)
	IntegerWrapper_BINARY_OVERLOAD(>>)
	IntegerWrapper_BINARY_OVERLOAD(<<)
#endif

#define IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(x) \
	WrappedT operator x(WrappedT input) const{ \
		auto temp = (WrappedT)*this; \
		temp x input; \
		return *this = temp; \
	}
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(+=)
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(-=)
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(*=)
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(/=)
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(%=)
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(&=)
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(|=)
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(^=)
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(<<=)
	IntegerWrapper_SPECIAL_ASSIGNMENT_OVERLOAD(>>=)
};
#endif

template <typename T, size_t S, typename = void>
struct WrapperSelector{
};

template <typename T, size_t S>
struct WrapperSelector<T, S, typename std::enable_if<std::is_integral<T>::value>::type>{
	typedef IntegerWrapper<T, S> type;
};

//#define WRAP_INTEGER_TYPE(x) \
//template <size_t S> \
//struct WrapperSelector<x, S>{ \
//	typedef IntegerWrapper<x, S> type; \
//}
//
//WRAP_INTEGER_TYPE(signed char);
//WRAP_INTEGER_TYPE(unsigned char);
//WRAP_INTEGER_TYPE(signed short);
//WRAP_INTEGER_TYPE(unsigned short);
//WRAP_INTEGER_TYPE(signed int);
//WRAP_INTEGER_TYPE(unsigned int);
//WRAP_INTEGER_TYPE(signed long);
//WRAP_INTEGER_TYPE(unsigned long);
//WRAP_INTEGER_TYPE(signed long long);
//WRAP_INTEGER_TYPE(unsigned long long);
//WRAP_INTEGER_TYPE(wchar_t);

template <typename WrappedT, typename MemoryT, size_t Size>
class EnumWrapper{
public:
	typedef typename WrapperSelector<MemoryT, Size>::type wrapper_type;
	typedef typename wrapper_type::callback_struct callback_struct;
	typedef WrappedT type;
	static const size_t size = Size;

	wrapper_type value;

	EnumWrapper(void *memory, const callback_struct &callbacks): value(memory, callbacks){}
	EnumWrapper(EnumWrapper &&other){
		this->value = std::move(other.value);
	}
	EnumWrapper(const EnumWrapper &other) = default;
	WrappedT operator=(const EnumWrapper &other) WRAPPER_CONST{
		auto ret = (WrappedT)other;
		*this = ret;
		return ret;
	}
	const EnumWrapper &operator=(EnumWrapper &&other) = delete;
	WrappedT operator=(WrappedT input) WRAPPER_CONST{
		this->value = (MemoryT)input;
		return input;
	}
	operator WrappedT() const{
		return (WrappedT)(MemoryT)this->value;
	}
	WrappedT enum_value() const{
		return (WrappedT)*this;
	}
};

template <typename T, size_t S>
struct WrapperSelector<T, S, typename std::enable_if<std::is_enum<T>::value>::type>{
	typedef EnumWrapper<T, byte_t, S> type;
};

//template <size_t S>
//struct WrapperSelector<NpcMovementDirection, S>{
//	typedef EnumWrapper<NpcMovementDirection, byte_t, S> type;
//};

template <typename WrappedT, size_t Length, size_t ElementSize>
class WrappedArray{
public:
	typedef typename WrapperSelector<WrappedT, ElementSize>::type indexed_type;
	typedef typename indexed_type::callback_struct callback_struct;
	static const size_t length = Length;
	static const size_t size = Length * ElementSize;
private:
	void *memory;
	callback_struct callbacks;

	void copy(const WrappedArray &other){
		this->memory = other.memory;
		this->read = other.read;
		this->write = other.write;
	}
public:
	WrappedArray(void *memory, const callback_struct &callbacks): memory(memory), callbacks(callbacks){}
	WrappedArray(WrappedArray &&other){
		this->copy(other);
	}
	WrappedArray(const WrappedArray &other){
		this->copy(other);
	}
	void operator=(const WrappedArray &) = delete;
	void operator=(WrappedArray &&other) = delete;
	indexed_type operator[](size_t idx) WRAPPER_CONST{
		if (idx >= Length)
			array_overflow();
		return indexed_type((char *)this->memory + idx * ElementSize, callbacks);
	}
	template <typename WrappedT_it, size_t Length_it, size_t ElementSize_it>
	class Iterator{
		WRAPPER_CONST WrappedArray<WrappedT_it, Length_it, ElementSize_it> *array;
		size_t index;
	public:
		typedef indexed_type value_type;
		typedef typename std::make_signed<size_t>::type difference_type;
		typedef value_type *pointer;
		typedef value_type &reference;
		typedef std::random_access_iterator_tag iterator_category;
		Iterator(): array(nullptr), index(0){}
		Iterator(WRAPPER_CONST WrappedArray<WrappedT_it, Length_it, ElementSize_it> *array, size_t index): array(array), index(index){}
		Iterator(const Iterator &it) = default;
		Iterator(Iterator &&it){
			*this = it;
		}
		const Iterator &operator=(const Iterator &it){
			this->array = it.array;
			this->index = it.index;
			return *this;
		}
		Iterator operator++(){
			this->index++;
			return *this;
		}
		Iterator operator--(){
			this->index--;
			return *this;
		}
		Iterator operator++(int){
			auto ret = *this;
			this->index++;
			return ret;
		}
		Iterator operator--(int){
			auto ret = *this;
			this->index--;
			return ret;
		}
		indexed_type operator*() WRAPPER_CONST{
			return (*this->array)[this->index];
		}
		indexed_type operator[](difference_type n) WRAPPER_CONST{
			return (*this->array)[this->index + n];
		}
		Iterator operator+(difference_type n) const{
			auto ret = *this;
			ret.index += n;
			return ret;
		}
		Iterator operator-(difference_type n) const{
			auto ret = *this;
			ret.index -= n;
			return ret;
		}
		difference_type operator-(Iterator other) const{
			if (this->array != other.array)
				throw std::runtime_error("Incorrect iterator usage.");
			return (difference_type)this->index - (difference_type)other.index;
		}
		Iterator operator+=(difference_type n){
			this->index += n;
			return *this;
		}
		Iterator operator-=(difference_type n){
			this->index -= n;
			return *this;
		}
		bool operator==(const Iterator &other) const{
			if (this->array != other.array)
				throw std::runtime_error("Incorrect iterator usage.");
			return this->index == other.index;
		}
		bool operator!=(const Iterator &other) const{
			return !(*this == other);
		}
		bool operator<(const Iterator &other) const{
			if (this->array != other.array)
				throw std::runtime_error("Incorrect iterator usage.");
			return this->index < other.index;
		}
	};
	typedef Iterator<WrappedT, Length, ElementSize> iterator;
	iterator begin() WRAPPER_CONST{
		return iterator(this, 0);
	}
	iterator end() WRAPPER_CONST{
		return iterator(this, Length);
	}
	void fill(const WrappedT &value) WRAPPER_CONST{
		for (auto &it : *this)
			it = value;
	}
	std::enable_if<std::is_same<WrappedT, byte_t>::value, void>
	operator=(const std::string &string) WRAPPER_CONST{
		auto n = std::min(string.size(), Length);
		for (size_t i = 0; i < n; i++)
			(*this)[i] = string[i];
		return std::enable_if<std::is_same<WrappedT, byte_t>::value, void>();
	}
	std::enable_if<std::is_same<WrappedT, byte_t>::value, std::string>
	to_string() const{
		std::string ret;
		ret.reserve(Length);
		for (auto &it : *this){
			if (!it)
				break;
			ret.push_back(it);
		}
		return ret;
	}
};

class MovementFlags{
public:
	typedef typename WrapperSelector<std::uint8_t, 1>::type member_type;
	typedef member_type::callback_struct callback_struct;
private:
	member_type data;
public:
	MovementFlags(void *memory, const callback_struct &callbacks):
		data(memory, callbacks)
	{}
	MovementFlags(const MovementFlags &) = default;
	MovementFlags(MovementFlags &&) = delete;
	void operator=(const MovementFlags &) = delete;
	void operator=(MovementFlags &&) = delete;
	MovementStatus get_movement_status() const;
	void set_movement_status(MovementStatus);
	bool get_face_player() const;
	void set_face_player(bool);
	void clear(){
		this->data = 0;
	}
};

class SpriteStateData1{
public:
	typedef typename WrapperSelector<std::uint8_t, 1>::type member_type;
	typedef typename EnumWrapper<SpriteFacingDirection, byte_t, 1> direction_type;
	struct callback_struct{
		typename member_type::callback_struct cb1;
		typename direction_type::callback_struct cb2;
		typename MovementFlags::callback_struct cb3;
	};
	static const size_t size = 16;
private:
public:
	//Offset: 0
	member_type picture_id;
	//Offset: 1
	MovementFlags movement_status;
	//Offset: 2
	member_type sprite_image_idx;
	//Offset: 3
	member_type y_step_vector;
	//Offset: 4
	member_type y_pixels;
	//Offset: 5
	member_type x_step_vector;
	//Offset: 6
	member_type x_pixels;
	//Offset: 7
	member_type intra_anim_frame_counter;
	//Offset: 8
	member_type anim_frame_counter;
	//Offset: 9
	direction_type facing_direction;

	SpriteStateData1(void *memory, const callback_struct &callbacks):
			picture_id              ((char *)memory + 0, callbacks.cb1),
			movement_status         ((char *)memory + 1, callbacks.cb3),
			sprite_image_idx        ((char *)memory + 2, callbacks.cb1),
			y_step_vector           ((char *)memory + 3, callbacks.cb1),
			y_pixels                ((char *)memory + 4, callbacks.cb1),
			x_step_vector           ((char *)memory + 5, callbacks.cb1),
			x_pixels                ((char *)memory + 6, callbacks.cb1),
			intra_anim_frame_counter((char *)memory + 7, callbacks.cb1),
			anim_frame_counter      ((char *)memory + 8, callbacks.cb1),
			facing_direction        ((char *)memory + 9, callbacks.cb2)
	{}
	SpriteStateData1(const SpriteStateData1 &other):
			picture_id              (other.picture_id),
			movement_status         (other.movement_status),
			sprite_image_idx        (other.sprite_image_idx),
			y_step_vector           (other.y_step_vector),
			y_pixels                (other.y_pixels),
			x_step_vector           (other.x_step_vector),
			x_pixels                (other.x_pixels),
			intra_anim_frame_counter(other.intra_anim_frame_counter),
			anim_frame_counter      (other.anim_frame_counter),
			facing_direction        (other.facing_direction)
	{}
	SpriteStateData1(SpriteStateData1 &&other):
		SpriteStateData1((const SpriteStateData1 &)other)
	{}
	//SpriteStateData1(SpriteStateData1 &&other) = delete;
	void operator=(const SpriteStateData1 &) = delete;
	void operator=(SpriteStateData1 &&) = delete;
};

class SpriteStateData2{
public:
	typedef typename WrapperSelector<std::uint8_t, 1>::type member_type;
	typedef typename member_type::callback_struct callback_struct;
	static const size_t size = 16;
private:
public:
	//Offset: 0
	member_type walk_animation_counter;
	//Offset: 2
	member_type y_displacement;
	//Offset: 3
	member_type x_displacement;
	//Offset: 4
	member_type map_y;
	//Offset: 5
	member_type map_x;
	//Offset: 6
	member_type movement_byte1;
	//Offset: 7
	member_type grass_priority;
	//Offset: 8
	member_type movement_delay;
	//Offset: 14 (0x0E)
	member_type sprite_image_base_offset;

	SpriteStateData2(void *memory, const callback_struct &callbacks):
		walk_animation_counter  ((char *)memory +  0, callbacks),
		y_displacement          ((char *)memory +  2, callbacks),
		x_displacement          ((char *)memory +  3, callbacks),
		map_y                   ((char *)memory +  4, callbacks),
		map_x                   ((char *)memory +  5, callbacks),
		movement_byte1          ((char *)memory +  6, callbacks),
		grass_priority          ((char *)memory +  7, callbacks),
		movement_delay          ((char *)memory +  8, callbacks),
		sprite_image_base_offset((char *)memory + 14, callbacks)
	{}
	SpriteStateData2(const SpriteStateData2 &other):
		walk_animation_counter  (other.walk_animation_counter),
		y_displacement          (other.y_displacement),
		x_displacement          (other.x_displacement),
		map_y                   (other.map_y),
		map_x                   (other.map_x),
		movement_byte1          (other.movement_byte1),
		grass_priority          (other.grass_priority),
		movement_delay          (other.movement_delay),
		sprite_image_base_offset(other.sprite_image_base_offset)
	{}
	SpriteStateData2(SpriteStateData2 &&other):
		SpriteStateData2((const SpriteStateData2 &)other)
	{}
	//SpriteStateData2(SpriteStateData2 &&other) = delete;
	void operator=(const SpriteStateData2 &) = delete;
	void operator=(SpriteStateData2 &&) = delete;
};

class PcBox{
public:
	typedef typename WrapperSelector<std::uint8_t, 1>::type u8_type;
	typedef typename WrapperSelector<std::uint16_t, 2>::type u16_type;
	typedef typename WrapperSelector<std::uint32_t, 3>::type u24_type;
	struct callback_structs{
		u8_type::callback_struct cb8;
		u16_type::callback_struct cb16;
		u24_type::callback_struct cb24;
	};
	static const size_t size = 16;
private:
public:
	//Offset: 0
	u8_type species;
	//Offset: 1
	u16_type hp;
	//Offset: 3
	u8_type box_level;
	//u8_type party_pos;
	//Offset: 4
	u8_type status;
	//Offset: 5
	u8_type type1;
	//Offset: 6
	u8_type type2;
	//Offset: 7
	u8_type catch_rate;
	//Offset: 8
	WrappedArray<typename u8_type::type, 4, u8_type::size> moves;
	//Offset: 12 (0x0C)
	u16_type original_trainer_id;
	//Offset: 14 (0x0E)
	u24_type experience;
	//Offset: 17 (0x11)
	u16_type hp_xp;
	//Offset: 19 (0x13)
	u16_type attack_xp;
	//Offset: 21 (0x15)
	u16_type defense_xp;
	//Offset: 23 (0x17)
	u16_type speed_xp;
	//Offset: 25 (0x19)
	u16_type special_xp;
	//Offset: 27 (0x1B)
	WrappedArray<typename u8_type::type, 2, u8_type::size> dvs;
	//Offset: 29 (0x1D)
	WrappedArray<typename u8_type::type, 4, u8_type::size> pp;

	PcBox(void *memory, const callback_structs &callbacks):
		species            ((char *)memory +  0, callbacks.cb8),
		hp                 ((char *)memory +  1, callbacks.cb16),
		box_level          ((char *)memory +  3, callbacks.cb8),
		/*
		party_pos          ((char *)memory +  3, callbacks.cb8),
		*/
		status             ((char *)memory +  4, callbacks.cb8),
		type1              ((char *)memory +  5, callbacks.cb8),
		type2              ((char *)memory +  6, callbacks.cb8),
		catch_rate         ((char *)memory +  7, callbacks.cb8),
		moves              ((char *)memory +  8, callbacks.cb8),
		original_trainer_id((char *)memory + 12, callbacks.cb16),
		experience         ((char *)memory + 14, callbacks.cb24),
		hp_xp              ((char *)memory + 17, callbacks.cb16),
		attack_xp          ((char *)memory + 19, callbacks.cb16),
		defense_xp         ((char *)memory + 21, callbacks.cb16),
		speed_xp           ((char *)memory + 23, callbacks.cb16),
		special_xp         ((char *)memory + 25, callbacks.cb16),
		dvs                ((char *)memory + 27, callbacks.cb8),
		pp                 ((char *)memory + 29, callbacks.cb8)
	{}
	PcBox(const PcBox &) = default;
	PcBox(PcBox &&) = delete;
	void operator=(const PcBox &) = delete;
	void operator=(PcBox &&) = delete;
};

struct OptionsTuple{
	bool battle_animations_enabled;
	BattleStyle battle_style;
	TextSpeed text_speed;
};

class UserOptions{
public:
	typedef typename WrapperSelector<std::uint8_t, 1>::type member_type;
	typedef typename member_type::callback_struct callback_struct;
	static const size_t size = 1;
private:
	member_type options;
public:
	UserOptions(void *memory, const callback_struct &callbacks):
		options((char *)memory, callbacks)
	{}
	UserOptions(const UserOptions &) = default;
	UserOptions(UserOptions &&) = delete;
	void operator=(const UserOptions &) = delete;
	void operator=(UserOptions &&) = delete;

	bool get_battle_animation_enabled() const;
	void set_battle_animation_enabled(bool);
	BattleStyle get_battle_style() const;
	void set_battle_style(BattleStyle);
	TextSpeed get_text_speed() const;
	void set_text_speed(TextSpeed);
	operator OptionsTuple() const;
	void operator=(const OptionsTuple &);
};

class MapSpriteData{
public:
	typedef typename WrapperSelector<std::uint8_t, 1>::type member_type;
	typedef typename member_type::callback_struct callback_struct;
	static const size_t size = 2;
private:
public:
	//Offset: 0
	member_type movement_byte_2;
	//Offset: 1
	member_type text_id;
	MapSpriteData(void *memory, const callback_struct &callbacks):
		movement_byte_2((char *)memory, callbacks),
		text_id((char *)memory + 1, callbacks)
	{}
	MapSpriteData(const MapSpriteData &other) = default;
	MapSpriteData(MapSpriteData &&other): MapSpriteData((const MapSpriteData &)other){}
	void operator=(const MapSpriteData &) = delete;
	void operator=(MapSpriteData &&) = delete;
};

class MissableObject{
public:
	typedef typename WrapperSelector<std::uint8_t, 1>::type member_type;
	typedef typename member_type::callback_struct callback_struct;
	static const size_t size = 2;
private:
public:
	//Offset: 0
	member_type sprite_id;
	//Offset: 1
	member_type missable_object_index;
	MissableObject(void *memory, const callback_struct &callbacks):
		sprite_id((char *)memory, callbacks),
		missable_object_index((char *)memory + 1, callbacks)
	{}
	MissableObject(const MissableObject &other) = default;
	MissableObject(MissableObject &&other): MissableObject((const MissableObject &)other){}
	void operator=(const MapSpriteData &) = delete;
	void operator=(MapSpriteData &&) = delete;
};

#include "../CodeGeneration/output/bitmaps.h"

template <typename T, size_t S>
struct WrapperSelector<T, S, typename std::enable_if<std::is_class<T>::value>::type>{
	typedef T type;
};

//template <size_t S>
//struct WrapperSelector<SpriteStateData1, S>{
//	typedef SpriteStateData1 type;
//};
//
//template <size_t S>
//struct WrapperSelector<SpriteStateData2, S>{
//	typedef SpriteStateData2 type;
//};
//
//template <size_t S>
//struct WrapperSelector<PcBox, S>{
//	typedef PcBox type;
//};
