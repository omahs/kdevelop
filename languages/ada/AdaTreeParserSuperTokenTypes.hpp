#ifndef INC_AdaTreeParserSuperTokenTypes_hpp_
#define INC_AdaTreeParserSuperTokenTypes_hpp_

/* $ANTLR 2.7.7 (20061129): "ada.tree.g" -> "AdaTreeParserSuperTokenTypes.hpp"$ */

#ifndef CUSTOM_API
# define CUSTOM_API
#endif

#ifdef __cplusplus
struct CUSTOM_API AdaTreeParserSuperTokenTypes {
#endif
	enum {
		EOF_ = 1,
		PRAGMA = 4,
		IDENTIFIER = 5,
		SEMI = 6,
		LPAREN = 7,
		COMMA = 8,
		RPAREN = 9,
		RIGHT_SHAFT = 10,
		WITH = 11,
		DOT = 12,
		USE = 13,
		TYPE = 14,
		TIC = 15,
		RANGE = 16,
		DIGITS = 17,
		DELTA = 18,
		ACCESS = 19,
		PRIVATE = 20,
		PACKAGE = 21,
		BODY = 22,
		IS = 23,
		PROCEDURE = 24,
		FUNCTION = 25,
		NEW = 26,
		OTHERS = 27,
		PIPE = 28,
		DOT_DOT = 29,
		ALL = 30,
		COLON = 31,
		IN = 32,
		OUT = 33,
		RENAMES = 34,
		CHARACTER_LITERAL = 35,
		CHAR_STRING = 36,
		NuLL = 37,
		RECORD = 38,
		SEPARATE = 39,
		ABSTRACT = 40,
		RETURN = 41,
		TASK = 42,
		PROTECTED = 43,
		BOX = 44,
		ASSIGN = 45,
		ENTRY = 46,
		FOR = 47,
		END = 48,
		AT = 49,
		MOD = 50,
		SUBTYPE = 51,
		EXCEPTION = 52,
		CONSTANT = 53,
		ARRAY = 54,
		OF = 55,
		ALIASED = 56,
		CASE = 57,
		WHEN = 58,
		TAGGED = 59,
		LIMITED = 60,
		GENERIC = 61,
		BEGIN = 62,
		LT_LT = 63,
		GT_GT = 64,
		IF = 65,
		THEN = 66,
		ELSIF = 67,
		ELSE = 68,
		LOOP = 69,
		WHILE = 70,
		REVERSE = 71,
		DECLARE = 72,
		EXIT = 73,
		GOTO = 74,
		ACCEPT = 75,
		DO = 76,
		DELAY = 77,
		UNTIL = 78,
		SELECT = 79,
		ABORT = 80,
		OR = 81,
		TERMINATE = 82,
		RAISE = 83,
		REQUEUE = 84,
		AND = 85,
		XOR = 86,
		NOT = 87,
		EQ = 88,
		NE = 89,
		LT_ = 90,
		LE = 91,
		GT = 92,
		GE = 93,
		PLUS = 94,
		MINUS = 95,
		CONCAT = 96,
		STAR = 97,
		DIV = 98,
		REM = 99,
		ABS = 100,
		EXPON = 101,
		NUMERIC_LIT = 102,
		ABORTABLE_PART = 103,
		ABORT_STATEMENT = 104,
		ABSTRACT_SUBPROGRAM_DECLARATION = 105,
		ACCEPT_ALTERNATIVE = 106,
		ACCEPT_STATEMENT = 107,
		ALLOCATOR = 108,
		ASSIGNMENT_STATEMENT = 109,
		ASYNCHRONOUS_SELECT = 110,
		ATTRIBUTE_DEFINITION_CLAUSE = 111,
		AT_CLAUSE = 112,
		BLOCK_STATEMENT = 113,
		CASE_STATEMENT = 114,
		CASE_STATEMENT_ALTERNATIVE = 115,
		CODE_STATEMENT = 116,
		COMPONENT_DECLARATION = 117,
		COMPONENT_LIST = 118,
		CONDITION = 119,
		CONDITIONAL_ENTRY_CALL = 120,
		CONTEXT_CLAUSE = 121,
		DECLARATIVE_ITEM = 122,
		DECLARATIVE_PART = 123,
		DEFINING_IDENTIFIER_LIST = 124,
		DELAY_ALTERNATIVE = 125,
		DELAY_STATEMENT = 126,
		DELTA_CONSTRAINT = 127,
		DIGITS_CONSTRAINT = 128,
		DISCRETE_RANGE = 129,
		DISCRIMINANT_ASSOCIATION = 130,
		DISCRIMINANT_CONSTRAINT = 131,
		DISCRIMINANT_SPECIFICATION = 132,
		ENTRY_BODY = 133,
		ENTRY_CALL_ALTERNATIVE = 134,
		ENTRY_CALL_STATEMENT = 135,
		ENTRY_DECLARATION = 136,
		ENTRY_INDEX_SPECIFICATION = 137,
		ENUMERATION_REPESENTATION_CLAUSE = 138,
		EXCEPTION_DECLARATION = 139,
		EXCEPTION_HANDLER = 140,
		EXCEPTION_RENAMING_DECLARATION = 141,
		EXIT_STATEMENT = 142,
		FORMAL_PACKAGE_DECLARATION = 143,
		FORMAL_TYPE_DECLARATION = 144,
		FULL_TYPE_DECLARATION = 145,
		GENERIC_FORMAL_PART = 146,
		GENERIC_INSTANTIATION = 147,
		GENERIC_PACKAGE_DECLARATION = 148,
		GENERIC_RENAMING_DECLARATION = 149,
		GENERIC_SUBPROGRAM_DECLARATION = 150,
		GOTO_STATEMENT = 151,
		HANDLED_SEQUENCE_OF_STATEMENTS = 152,
		IF_STATEMENT = 153,
		INCOMPLETE_TYPE_DECLARATION = 154,
		INDEXED_COMPONENT = 155,
		INDEX_CONSTRAINT = 156,
		LIBRARY_ITEM = 157,
		LOOP_STATEMENT = 158,
		NAME = 159,
		NULL_STATEMENT = 160,
		NUMBER_DECLARATION = 161,
		OBJECT_DECLARATION = 162,
		OBJECT_RENAMING_DECLARATION = 163,
		OPERATOR_SYMBOL = 164,
		PACKAGE_BODY = 165,
		PACKAGE_BODY_STUB = 166,
		PACKAGE_RENAMING_DECLARATION = 167,
		PACKAGE_SPECIFICATION = 168,
		PARAMETER_SPECIFICATION = 169,
		PREFIX = 170,
		PRIMARY = 171,
		PRIVATE_EXTENSION_DECLARATION = 172,
		PRIVATE_TYPE_DECLARATION = 173,
		PROCEDURE_CALL_STATEMENT = 174,
		PROTECTED_BODY = 175,
		PROTECTED_BODY_STUB = 176,
		PROTECTED_TYPE_DECLARATION = 177,
		RAISE_STATEMENT = 178,
		RANGE_ATTRIBUTE_REFERENCE = 179,
		RECORD_REPRESENTATION_CLAUSE = 180,
		REQUEUE_STATEMENT = 181,
		RETURN_STATEMENT = 182,
		SELECTIVE_ACCEPT = 183,
		SELECT_ALTERNATIVE = 184,
		SELECT_STATEMENT = 185,
		SEQUENCE_OF_STATEMENTS = 186,
		SINGLE_PROTECTED_DECLARATION = 187,
		SINGLE_TASK_DECLARATION = 188,
		STATEMENT = 189,
		SUBPROGRAM_BODY = 190,
		SUBPROGRAM_BODY_STUB = 191,
		SUBPROGRAM_DECLARATION = 192,
		SUBPROGRAM_RENAMING_DECLARATION = 193,
		SUBTYPE_DECLARATION = 194,
		SUBTYPE_INDICATION = 195,
		SUBTYPE_MARK = 196,
		SUBUNIT = 197,
		TASK_BODY = 198,
		TASK_BODY_STUB = 199,
		TASK_TYPE_DECLARATION = 200,
		TERMINATE_ALTERNATIVE = 201,
		TIMED_ENTRY_CALL = 202,
		TRIGGERING_ALTERNATIVE = 203,
		TYPE_DECLARATION = 204,
		USE_CLAUSE = 205,
		USE_TYPE_CLAUSE = 206,
		VARIANT = 207,
		VARIANT_PART = 208,
		WITH_CLAUSE = 209,
		ABSTRACT_FUNCTION_DECLARATION = 210,
		ABSTRACT_PROCEDURE_DECLARATION = 211,
		ACCESS_TO_FUNCTION_DECLARATION = 212,
		ACCESS_TO_OBJECT_DECLARATION = 213,
		ACCESS_TO_PROCEDURE_DECLARATION = 214,
		ACCESS_TYPE_DECLARATION = 215,
		ARRAY_OBJECT_DECLARATION = 216,
		ARRAY_TYPE_DECLARATION = 217,
		AND_THEN = 218,
		BASIC_DECLARATIVE_ITEMS_OPT = 219,
		BLOCK_BODY = 220,
		BLOCK_BODY_OPT = 221,
		CALL_STATEMENT = 222,
		COMPONENT_CLAUSES_OPT = 223,
		COMPONENT_ITEMS = 224,
		COND_CLAUSE = 225,
		DECIMAL_FIXED_POINT_DECLARATION = 226,
		DECLARE_OPT = 227,
		DERIVED_RECORD_EXTENSION = 228,
		DERIVED_TYPE_DECLARATION = 229,
		DISCRETE_SUBTYPE_DEF_OPT = 230,
		DISCRIMINANT_SPECIFICATIONS = 231,
		DISCRIM_PART_OPT = 232,
		ELSE_OPT = 233,
		ELSIFS_OPT = 234,
		ENTRY_INDEX_OPT = 235,
		ENUMERATION_TYPE_DECLARATION = 236,
		EXCEPT_HANDLER_PART_OPT = 237,
		EXTENSION_OPT = 238,
		FLOATING_POINT_DECLARATION = 239,
		FORMAL_DECIMAL_FIXED_POINT_DECLARATION = 240,
		FORMAL_DISCRETE_TYPE_DECLARATION = 241,
		FORMAL_FLOATING_POINT_DECLARATION = 242,
		FORMAL_FUNCTION_DECLARATION = 243,
		FORMAL_MODULAR_TYPE_DECLARATION = 244,
		FORMAL_ORDINARY_DERIVED_TYPE_DECLARATION = 245,
		FORMAL_ORDINARY_FIXED_POINT_DECLARATION = 246,
		FORMAL_PART_OPT = 247,
		FORMAL_PRIVATE_EXTENSION_DECLARATION = 248,
		FORMAL_PRIVATE_TYPE_DECLARATION = 249,
		FORMAL_PROCEDURE_DECLARATION = 250,
		FORMAL_SIGNED_INTEGER_TYPE_DECLARATION = 251,
		FUNCTION_BODY = 252,
		FUNCTION_BODY_STUB = 253,
		FUNCTION_DECLARATION = 254,
		FUNCTION_RENAMING_DECLARATION = 255,
		GENERIC_FUNCTION_DECLARATION = 256,
		GENERIC_FUNCTION_INSTANTIATION = 257,
		GENERIC_FUNCTION_RENAMING = 258,
		GENERIC_PACKAGE_INSTANTIATION = 259,
		GENERIC_PACKAGE_RENAMING = 260,
		GENERIC_PROCEDURE_DECLARATION = 261,
		GENERIC_PROCEDURE_INSTANTIATION = 262,
		GENERIC_PROCEDURE_RENAMING = 263,
		GUARD_OPT = 264,
		IDENTIFIER_COLON_OPT = 265,
		INIT_OPT = 266,
		ITERATION_SCHEME_OPT = 267,
		LABEL_OPT = 268,
		MARK_WITH_CONSTRAINT = 269,
		MODIFIERS = 270,
		MODULAR_TYPE_DECLARATION = 271,
		MOD_CLAUSE_OPT = 272,
		NOT_IN = 273,
		ORDINARY_DERIVED_TYPE_DECLARATION = 274,
		ORDINARY_FIXED_POINT_DECLARATION = 275,
		OR_ELSE = 276,
		OR_SELECT_OPT = 277,
		PARENTHESIZED_PRIMARY = 278,
		PRIVATE_TASK_ITEMS_OPT = 279,
		PROCEDURE_BODY = 280,
		PROCEDURE_BODY_STUB = 281,
		PROCEDURE_DECLARATION = 282,
		PROCEDURE_RENAMING_DECLARATION = 283,
		PROT_MEMBER_DECLARATIONS = 284,
		PROT_OP_BODIES_OPT = 285,
		PROT_OP_DECLARATIONS = 286,
		RANGED_EXPRS = 287,
		RECORD_TYPE_DECLARATION = 288,
		SELECTOR_NAMES_OPT = 289,
		SIGNED_INTEGER_TYPE_DECLARATION = 290,
		TASK_ITEMS_OPT = 291,
		UNARY_MINUS = 292,
		UNARY_PLUS = 293,
		VALUE = 294,
		VALUES = 295,
		VARIANTS = 296,
		COMMENT_INTRO = 297,
		DIGIT = 298,
		EXPONENT = 299,
		EXTENDED_DIGIT = 300,
		BASED_INTEGER = 301,
		WS_ = 302,
		COMMENT = 303,
		CHARACTER_STRING = 304,
		NULL_TREE_LOOKAHEAD = 3
	};
#ifdef __cplusplus
};
#endif
#endif /*INC_AdaTreeParserSuperTokenTypes_hpp_*/
