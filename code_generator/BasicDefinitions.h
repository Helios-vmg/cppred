#include <cassert>

enum class Register8{
	B = 0,
	C = 1,
	D = 2,
	E = 3,
	H = 4,
	L = 5,
	None = 6,
	A = 7,
};

enum class Register16{
	AF = 0,
	BC = 1,
	DE = 2,
	HL = 3,
	SP = 4,
	PC = 5,
};

enum class Register16A{
	BC = 0,
	DE = 1,
	HL = 2,
	SP = 3,
};

enum class Register16B{
	BC = 0,
	DE = 1,
	HL = 2,
	AF = 3,
};

inline Register16 to_Register16(Register16A reg){
	switch (reg){
		case Register16A::BC:
			return Register16::BC;
		case Register16A::DE:
			return Register16::DE;
		case Register16A::HL:
			return Register16::HL;
		case Register16A::SP:
			return Register16::SP;
	}
	abort();
}

inline Register16 to_Register16(Register16B reg){
	switch (reg){
		case Register16B::AF:
			return Register16::AF;
		case Register16B::BC:
			return Register16::BC;
		case Register16B::DE:
			return Register16::DE;
		case Register16B::HL:
			return Register16::HL;
	}
	abort();
}

enum class ConditionalJumpType{
	NotZero = 0,
	Zero = 1,
	NotCarry = 2,
	Carry = 3,
};

enum class BitwiseOps{
	RotateLeftNotUsingCarry = 0,
	RotateRightNotUsingCarry = 1,
	RotateLeftUsingCarry = 2,
	RotateRightUsingCarry = 3,
	ShiftLeft = 4,
	ArithmeticShiftRight = 5,
	Swap = 6,
	BitwiseShiftRight = 7,
};

enum class BitfieldOps{
	BitCheck = 1,
	BitReset = 2,
	BitSet = 3,
};

enum class ProcessorFlags{
	Zero = 0,
	Subtract = 1,
	HalfCarry = 2,
	Carry = 3,
};
