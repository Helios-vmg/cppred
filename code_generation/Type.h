#pragma once

#include <memory>
#include <string>
#include <sstream>
#include <map>
#include <functional>

class Number{
public:
	virtual ~Number(){}
	virtual std::string to_string() const = 0;
	virtual std::unique_ptr<Number> operator*(const Number &) const = 0;
	virtual std::unique_ptr<Number> operator*(unsigned n) const = 0;
};

class IntegerLiteralNumber : public Number{
	unsigned value;
public:
	IntegerLiteralNumber(unsigned value): value(value){}
	IntegerLiteralNumber(const std::string &value): value(0){
		std::stringstream stream;
		stream << value;
		stream >> this->value;
	}
	virtual ~IntegerLiteralNumber(){}
	unsigned get_value() const{
		return this->value;
	}
	std::string to_string() const override{
		std::stringstream stream;
		stream << this->value;
		return stream.str();
	}
	std::unique_ptr<Number> operator*(const Number &n) const override{
		auto il = dynamic_cast<const IntegerLiteralNumber *>(&n);
		if (il)
			return std::make_unique<IntegerLiteralNumber>(this->value * il->get_value());
		return n * *this;
	}
	std::unique_ptr<Number> operator*(unsigned n) const override{
		return std::make_unique<IntegerLiteralNumber>(this->value * n);
	}
};

class CompoundNumber : public Number{
	std::string value;
public:
	CompoundNumber(const std::string &value): value(value){}
	virtual ~CompoundNumber(){}
	std::string to_string() const override{
		return this->value;
	}
	std::unique_ptr<Number> operator*(const Number &n) const override{
		std::stringstream new_value;
		new_value << '(' << this->value << ") * " << n.to_string();
		return std::make_unique<CompoundNumber>(new_value.str());
	}
	std::unique_ptr<Number> operator*(unsigned n) const override{
		std::stringstream new_value;
		new_value << '(' << this->value << ") * " << n;
		return std::make_unique<CompoundNumber>(new_value.str());
	}
};

class Type{
public:
	std::string class_name;

	virtual ~Type(){}
	virtual std::unique_ptr<Number> get_size() = 0;
	virtual std::string generate_declarations(const std::string &name) const;
	virtual std::string generate_initializer(unsigned address, unsigned base_address, const std::unique_ptr<Number> &size, const std::string &name) const;
	virtual std::string get_actual_type_name() const = 0;
	virtual std::string get_virtual_type_name() const{
		return this->get_actual_type_name();
	}
	virtual std::string get_callback_struct() const = 0;
};

class TypeUint : public Type{
protected:
	unsigned N;
	bool big_endian;
public:
	TypeUint(unsigned n, bool big_endian = false): N(n), big_endian(big_endian){}
	virtual ~TypeUint(){}
	std::unique_ptr<Number> get_size() override{
		return std::make_unique<IntegerLiteralNumber>(N);
	}
	virtual std::string get_actual_type_name() const override;
	virtual std::string get_virtual_type_name() const override;
	virtual std::string get_callback_struct() const override;
};

class TypeBcdInt : public Type{
	unsigned N;
public:
	TypeBcdInt(unsigned n): N(n){}
	~TypeBcdInt(){}
	std::unique_ptr<Number> get_size() override{
		return std::make_unique<IntegerLiteralNumber>(N);
	}
	std::string get_actual_type_name() const override;
	std::string get_virtual_type_name() const override{
		return "unsigned";
	}
	std::string get_callback_struct() const override;
};

class Pointer : public Type{
protected:
	bool big_endian;
public:
	Pointer(bool big_endian = false): big_endian(big_endian){}
	virtual ~Pointer(){}
	std::unique_ptr<Number> get_size() override{
		return std::make_unique<IntegerLiteralNumber>(2);
	}
	std::string get_actual_type_name() const override;
	std::string get_virtual_type_name() const override;
	std::string get_callback_struct() const override;
};

class DataPointer : public Pointer{
public:
	DataPointer(bool big_endian = false): Pointer(big_endian){}
	~DataPointer(){}
};

class CodePointer : public Pointer{
public:
	CodePointer(bool big_endian = false): Pointer(big_endian){}
	~CodePointer(){}
};

//------------------------------------------------------------------------------

class Struct : public Type{
public:
	virtual ~Struct(){}
	virtual std::string generate_initializer(unsigned address, unsigned base_address, const std::unique_ptr<Number> &size, const std::string &name) const override;
	virtual std::unique_ptr<Number> get_size() override{
		return std::make_unique<CompoundNumber>(this->get_actual_type_name() + "::size");
	}
};

class SpecialStruct : public Type{
public:
	virtual ~SpecialStruct(){}
	virtual std::string generate_initializer(unsigned address, unsigned base_address, const std::unique_ptr<Number> &size, const std::string &name) const override;
	virtual std::unique_ptr<Number> get_size() override{
		return std::make_unique<CompoundNumber>(this->get_actual_type_name() + "::size");
	}
};

class spritestatedata1Struct : public Struct{
public:
	virtual ~spritestatedata1Struct(){}
	virtual std::string get_actual_type_name() const override;
	virtual std::string get_callback_struct() const override;
};

class spritestatedata2Struct : public Struct{
public:
	virtual ~spritestatedata2Struct(){}
	virtual std::string get_actual_type_name() const override;
	virtual std::string get_callback_struct() const override;
};

class mapspritedataStruct : public spritestatedata2Struct{
public:
	virtual ~mapspritedataStruct(){}
	std::string get_actual_type_name() const override;
};

class missableobjectStruct : public mapspritedataStruct{
public:
	virtual ~missableobjectStruct(){}
	std::string get_actual_type_name() const override;
};

class PackedBitsWrapper : public Struct{
	std::string name;
public:
	PackedBitsWrapper(const std::string &name): name(name){}
	std::unique_ptr<Number> get_size() override{
		return std::make_unique<IntegerLiteralNumber>(1);
	}
	std::string get_actual_type_name() const override{
		return this->name;
	}
	std::string get_callback_struct() const override;
};

class pcboxmemberStruct : public Struct{
public:
	virtual ~pcboxmemberStruct(){}
	virtual std::string get_actual_type_name() const override;
	virtual std::string get_callback_struct() const override;
};

class partymemberStruct : public pcboxmemberStruct{
public:
	virtual ~partymemberStruct(){}
	std::string get_actual_type_name() const override;
};

class pcboxStruct : public pcboxmemberStruct{
public:
	virtual ~pcboxStruct(){}
	virtual std::string get_actual_type_name() const override;
};

class maindataStruct : public SpecialStruct{
public:
	virtual ~maindataStruct(){}
	virtual std::string get_actual_type_name() const override;
	virtual std::string get_callback_struct() const override;
};

class spritedataStruct : public SpecialStruct{
public:
	virtual ~spritedataStruct(){}
	virtual std::string get_actual_type_name() const override;
	virtual std::string get_callback_struct() const override;
};

class partydataStruct : public SpecialStruct{
public:
	virtual ~partydataStruct(){}
	virtual std::string get_actual_type_name() const override;
	virtual std::string get_callback_struct() const override;
};

class boxdataStruct : public SpecialStruct{
public:
	virtual ~boxdataStruct(){}
	virtual std::string get_actual_type_name() const override;
	virtual std::string get_callback_struct() const override;
};

class spriteobjectStruct : public Struct{
public:
	virtual ~spriteobjectStruct(){}
	virtual std::string get_actual_type_name() const override;
	virtual std::string get_callback_struct() const override;
};

//------------------------------------------------------------------------------

class Array : public Struct{
	std::unique_ptr<Type> inner;
	std::unique_ptr<Number> length;
public:
	Array(std::unique_ptr<Type> &&inner, unsigned length): inner(std::move(inner)), length(std::make_unique<IntegerLiteralNumber>(length)){}
	Array(std::unique_ptr<Type> &&inner, std::unique_ptr<Number> &&length): inner(std::move(inner)), length(std::move(length)){}
	~Array(){}
	std::unique_ptr<Number> get_size() override{
		return *this->inner->get_size() * *this->length;
	}
	std::string get_actual_type_name() const override;
	std::string get_callback_struct() const override{
		return this->inner->get_callback_struct();
	}
};

class EnumUint : public TypeUint{
	std::string type_name;
public:
	EnumUint(const std::string &type_name, unsigned n, bool big_endian = false): TypeUint(n, big_endian), type_name(type_name){}
	std::string get_actual_type_name() const override;
	std::string get_virtual_type_name() const override{
		return this->type_name;
	}
};

typedef std::function<std::unique_ptr<Type>()> basic_type_constructor;
typedef std::map<std::string, basic_type_constructor> typemap_t;

std::unique_ptr<Type> parse_string_to_Type(const typemap_t &typemap, const std::string &);
typemap_t declare_default_types();
