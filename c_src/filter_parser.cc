#include "filter_parser.h"
#include <cstring>

template<typename T>
ExpressionNode<T>* parse_const_expr(ErlNifEnv* env, ERL_NIF_TERM operand, Extractor& ext) {
    printf("Called the wrong version of parce_constexpr\n");
    return NULL;
}

template<> ExpressionNode<int64_t>* parse_const_expr(ErlNifEnv* env, ERL_NIF_TERM operand, Extractor& ext) {
    int val;
    enif_get_int(env, operand, &val);
    return new ConstantValue<int64_t>(val);
}

template<typename T>
ExpressionNode<T>* parse_expression_node(ErlNifEnv* env, ERL_NIF_TERM root, Extractor& ext) {
    char op[20];
    const ERL_NIF_TERM* op_args;
    int arity;
    if (enif_get_tuple(env, root, &arity, &op_args) && arity==2) {
        if (enif_get_atom(env, op_args[0], op, sizeof(op), ERL_NIF_LATIN1)) {
            if (strcmp(op, eleveldb::filter::CONST_OP)==0) {
                return parse_const_expr<T>(env, op_args[1], ext);
            }
            if (strcmp(op, eleveldb::filter::FIELD_OP)==0) {
                return parse_field_expr<T>(env, op_args[1], ext);
            }
        }
    }
    return NULL;
}

template<> ExpressionNode<bool>* parse_expression_node<bool>(ErlNifEnv* env, ERL_NIF_TERM root, Extractor& ext) {
char op[20];
    const ERL_NIF_TERM* op_args;
    int arity;
    if (enif_get_tuple(env, root, &arity, &op_args) && arity==2) {
        if (enif_get_string(env, op_args[0], op, sizeof(op), ERL_NIF_LATIN1)) {
            if (strcmp(op, eleveldb::filter::EQ_OP)==0) {
                return parse_equals_expr(env, op_args[1], ext);
            }
            else if (strcmp(op, eleveldb::filter::LTE_OP)==0) {
                return parse_lte_expr(env, op_args[1], ext);
            }
            else if (strcmp(op, eleveldb::filter::GTE_OP)==0) {
                return parse_gte_expr(env, op_args[1], ext);
            }
            else if (strcmp(op, eleveldb::filter::AND_OP)==0) {
                return parse_and_expr(env, op_args[1], ext);
            }
        }
    }
    return NULL;
}

ExpressionNode<bool>* parse_equals_expr(ErlNifEnv* env, ERL_NIF_TERM operands, Extractor& ext) {
    unsigned int oplen;
    ERL_NIF_TERM lhs, rhs, rest = operands;
    if (enif_get_list_length(env, operands, &oplen) && oplen==2) {
        if (enif_get_list_cell(env, rest, &lhs, &rest) &&
                enif_get_list_cell(env, rest, &rhs, &rest)) {
            return new EqOperator<int64_t>(parse_expression_node<int64_t>(env, lhs, ext),
                parse_expression_node<int64_t>(env, rhs, ext));
        }
    }
    return NULL;
}

ExpressionNode<bool>* parse_lte_expr(ErlNifEnv* env, ERL_NIF_TERM operands, Extractor& ext) {
    unsigned int oplen;
    ERL_NIF_TERM lhs, rhs, rest = operands;
    if (enif_get_list_length(env, operands, &oplen) && oplen==2) {
        if (enif_get_list_cell(env, rest, &lhs, &rest) &&
                enif_get_list_cell(env, rest, &rhs, &rest)) {
            return new LteOperator<int64_t>(parse_expression_node<int64_t>(env, lhs, ext),
                parse_expression_node<int64_t>(env, rhs, ext));
        }
    }
    return NULL;
}

ExpressionNode<bool>* parse_gte_expr(ErlNifEnv* env, ERL_NIF_TERM operands, Extractor& ext) {
    unsigned int oplen;
    ERL_NIF_TERM lhs, rhs, rest = operands;
    if (enif_get_list_length(env, operands, &oplen) && oplen==2) {
        if (enif_get_list_cell(env, rest, &lhs, &rest) &&
                enif_get_list_cell(env, rest, &rhs, &rest)) {
            return new GteOperator<int64_t>(parse_expression_node<int64_t>(env, lhs, ext),
                parse_expression_node<int64_t>(env, rhs, ext));
        }
    }
    return NULL;
}

ExpressionNode<bool>* parse_and_expr(ErlNifEnv* env, ERL_NIF_TERM operands, Extractor& ext) {
    unsigned int oplen;
   ERL_NIF_TERM lhs, rhs, rest = operands;
    if (enif_get_list_length(env, operands, &oplen) && oplen==2) {
        if (enif_get_list_cell(env, rest, &lhs, &rest) &&
                enif_get_list_cell(env, rest, &rhs, &rest)) {
            return new AndOperator(parse_expression_node<bool>(env, lhs, ext),
                parse_expression_node<bool>(env, rhs, ext));
        }
    }
    return NULL;
}

template<typename T>
ExpressionNode<T>* parse_field_expr(ErlNifEnv* env, ERL_NIF_TERM operand, Extractor& ext) {
    char field_name[255];
    if (enif_get_string(env, operand, field_name, sizeof(field_name), ERL_NIF_LATIN1)) {
        ext.add_field(field_name);
        return new FieldValue<T>(field_name);
    }
    return NULL;
}

ExpressionNode<bool>* parse_range_filter_opts(ErlNifEnv* env, ERL_NIF_TERM options, Extractor& ext) {
    return parse_expression_node<bool>(env, options, ext);
}
