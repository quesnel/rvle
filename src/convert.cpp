/**
 * @file convert.cpp
 * @author The VLE Development Team
 */

/*
 * VLE Environment - the multimodeling and simulation environment
 * This file is a part of the VLE environment (http://vle.univ-littoral.fr)
 * Copyright (C) 2003 - 2012 The VLE Development Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <memory>
#include <string>
#include <vle/value/Boolean.hpp>
#include <vle/value/Double.hpp>
#include <vle/value/Integer.hpp>
#include <vle/value/Map.hpp>
#include <vle/value/Matrix.hpp>
#include <vle/value/Null.hpp>
#include <vle/value/Set.hpp>
#include <vle/value/String.hpp>
#include <vle/value/Table.hpp>
#include <vle/value/Tuple.hpp>
#include <vle/value/XML.hpp>

#ifdef ENABLE_NLS
#undef ENABLE_NLS
#endif

#include "convert.h"
#include "rvle.h"
#include <Rdefines.h>

using namespace vle;

value::Value::type
rvle_get_vector_type(const value::Matrix& mat,
                     unsigned int col,
                     bool avoidFirstLine)
{
    for (unsigned int r = 0; r < mat.rows(); r++) {
        if ((r > 0) or (not avoidFirstLine)) {
            const std::unique_ptr<value::Value>& v = mat.get(col, r);
            if (v) {
                return v->getType();
            }
        }
    }
    return value::Value::DOUBLE;
}

value::Value*
rvle_toVleValue_implicit(SEXP rval, const std::string& rvalClass)
{
    value::Value* res = 0;
    if (rvalClass == "data.frame") {
        SEXP temp, names, colData, elt;
        PROTECT(names = GET_NAMES(rval));
        PROTECT(temp = AS_LIST(rval));

        for (R_len_t col = 0; col < length(temp); col++) {
            PROTECT(colData = VECTOR_ELT(temp, col));
            if (col == 0) {
                res =
                  new value::Matrix(length(temp), length(colData) + 1, 10, 10);
            }
            res->toMatrix().set(col,
                                0,
                                value::String::create(
                                  std::string(CHAR(STRING_ELT(names, col)))));
            for (R_len_t row = 0; row < length(colData); row++) {
                PROTECT(elt = VECTOR_ELT(AS_LIST(colData), row));
                res->toMatrix().set(
                  col,
                  row + 1,
                  std::unique_ptr<value::Value>(
                    reinterpret_cast<value::Value*>(rvle_toVleValue(elt))));
                UNPROTECT(1); // elt
            }
            UNPROTECT(1); // colData
        }
        UNPROTECT(1); // temp
        UNPROTECT(1); // names
    }
    // TODO handle 'factors'
    return res;
}

value::Value*
rvle_toVleValue_implicit(SEXP rval)
{
    value::Value* res = 0;
    switch (TYPEOF(rval)) {
    case NILSXP:
        break;
    case SYMSXP:
        break;
    case LISTSXP:
        break;
    case CLOSXP:
        break;
    case ENVSXP:
        break;
    case PROMSXP:
        break;
    case LANGSXP:
        break;
    case SPECIALSXP:
        break;
    case BUILTINSXP:
        break;
    case CHARSXP:
        break;
    case LGLSXP: { // logical vectors
        res = new value::Set();
        SEXP temp;
        PROTECT(temp = AS_LOGICAL(rval));
        if (length(temp) == 1) {
            res = new value::Boolean(((bool)LOGICAL(temp)[0]));
        } else {
            for (R_len_t i = 0; i < length(temp); i++) {
                res->toSet().add(
                  value::Boolean::create(((bool)LOGICAL(temp)[i])));
            }
        }
        UNPROTECT(1);
        break;
    }
    case INTSXP: { // integer vectors
        SEXP temp, dimensions;
        PROTECT(temp = AS_INTEGER(rval));
        if (length(temp) == 1) {
            res = new value::Integer((int)INTEGER(temp)[0]);
        } else {
            PROTECT(dimensions = GET_DIM(temp));
            int nbDims = GET_LENGTH(dimensions);
            if (nbDims == 2) {
                int* nbDimsInt = INTEGER_POINTER(dimensions);
                int nrow = nbDimsInt[0];
                int ncol = nbDimsInt[1];
                res = new value::Table(ncol, nrow);
                for (int i = 0; i < ncol; i++) {
                    for (int j = 0; j < nrow; j++) {
                        res->toTable().get(i, j) = INTEGER(temp)[j + i * nrow];
                    }
                }
            } else {
                res = new value::Tuple();
                for (R_len_t i = 0; i < length(temp); i++) {
                    res->toTuple().add(((double)INTEGER(temp)[i]));
                }
            }
            UNPROTECT(1); // dimensions
        }
        UNPROTECT(1); // temp
        break;
    }
    case REALSXP: { //  numeric vectors
        SEXP temp, dimensions;
        int lengthSexp = 0;
        PROTECT(temp = AS_NUMERIC(rval));
        lengthSexp = length(temp);
        if (lengthSexp == 1) {
            res = new value::Double((double)REAL(temp)[0]);
        } else {
            PROTECT(dimensions = GET_DIM(temp));
            int nbDims = GET_LENGTH(dimensions);
            if (nbDims == 2) {
                int* nbDimsInt = INTEGER_POINTER(dimensions);
                int nrow = nbDimsInt[0];
                int ncol = nbDimsInt[1];
                res = new value::Table(ncol, nrow);
                for (int i = 0; i < ncol; i++) {
                    for (int j = 0; j < nrow; j++) {
                        res->toTable().get(i, j) = REAL(temp)[j + i * nrow];
                    }
                }
            } else {
                res = new value::Tuple();
                for (R_len_t i = 0; i < length(temp); i++) {
                    res->toTuple().add((double)REAL(temp)[i]);
                }
            }
            UNPROTECT(1); // dimensions
        }
        UNPROTECT(1); // temp
        break;
    }
    case CPLXSXP:
        break;
    case STRSXP: { // character vectors
        SEXP temp;
        PROTECT(temp = AS_CHARACTER(rval));
        if (length(temp) == 1) {
            res = new value::String(CHAR(STRING_ELT(temp, 0)));
        } else {
            res = new value::Set();
            for (R_len_t i = 0; i < length(temp); i++) {
                res->toSet().add(value::String::create(
                  std::string(CHAR(STRING_ELT(temp, i)))));
            }
        }
        UNPROTECT(1);
        break;
    }
    case DOTSXP:
        break;
    case ANYSXP:
        break;
    case VECSXP: { // list (generic vector)
        std::string namei;
        SEXP names, temp;
        PROTECT(names = GET_NAMES(rval));
        PROTECT(temp = AS_LIST(rval));
        if (length(temp) == length(names)) { // value::Map
            res = new value::Map();
            for (R_len_t i = 0; i < length(temp); i++) {
                res->toMap().add(CHAR(STRING_ELT(names, i)),
                                 std::unique_ptr<value::Value>(
                                   reinterpret_cast<value::Value*>(
                                     rvle_toVleValue(VECTOR_ELT(temp, i)))));
            }
        } else { // value::Set
            res = new value::Set();
            for (R_len_t i = 0; i < length(temp); i++) {
                res->toSet().add(std::unique_ptr<value::Value>(
                  reinterpret_cast<value::Value*>(
                    rvle_toVleValue(VECTOR_ELT(temp, i)))));
            }
        }
        UNPROTECT(1); // temp
        UNPROTECT(1); // names
        break;
    }
    case EXPRSXP:
        break;
    case BCODESXP:
        break;
    case EXTPTRSXP:
        break;
    case WEAKREFSXP:
        break;
    case RAWSXP:
        break;
    case S4SXP:
        break;
    }
    return res;
}

///////////////////////////////////
//        public API
///////////////////////////////////

SEXP
rvle_convertCharToSEXP(char** val, unsigned int size)
{
    SEXP r; /* condition list result */
    unsigned int i;
    PROTECT(r = allocVector(STRSXP, size));
    if (size > 0) {
        for (i = 0; i < size; ++i) {
            SET_STRING_ELT(r, i, mkChar(val[i]));
        }

        for (i = 0; i < size; ++i) {
            free(val[i]);
        }
        free(val);
    }
    UNPROTECT(1);
    return r;
}

SEXP
rvle_convertIntToSEXP(int val)
{
    SEXP r;
    PROTECT(r = allocVector(INTSXP, 1));
    INTEGER(r)[0] = val;
    UNPROTECT(1);
    return r;
}

SEXP
rvle_toRvalue(rvle_value_t vlevalToCast,
              int without_class_names,
              int multiple_values,
              int unlist_multiple_values,
              int matrix_type,
              int matrix_type_depth)
{
    value::Value* vleval = reinterpret_cast<value::Value*>(vlevalToCast);

    SEXP res = R_NilValue;

    if (!vleval) {
        PROTECT(res = allocVector(REALSXP, 1));
        REAL(res)[0] = NA_REAL; // WARNING NILSXP can lead to seg faults
        if (without_class_names == 0) {
            SET_CLASS(res, mkString("VleNIL"));
        }
        UNPROTECT(1); // res
        return (res);
    }
    switch (vleval->getType()) {
    case value::Value::BOOLEAN: {
        PROTECT(res = NEW_LOGICAL(1));
        LOGICAL(res)[0] = (bool)vleval->toBoolean().value();
        // Note Boolean/logical is not given a class name because not ambiguous
        // handle by implicit conversion
        UNPROTECT(1);
        break;
    }
    case value::Value::DOUBLE: {
        PROTECT(res = allocVector(REALSXP, 1));
        REAL(res)[0] = (double)vleval->toDouble().value();
        // Note Double/numeric is not given a class name because not ambiguous
        // handle by implicit conversion
        UNPROTECT(1);
        break;
    }
    case value::Value::INTEGER: {
        PROTECT(res = NEW_INTEGER(1));
        INTEGER(res)[0] = (int)vleval->toInteger().value();
        // Note Integer/integer is not given a class name because not ambiguous
        // handle by implicit conversion
        UNPROTECT(1);
        break;
    }
    case value::Value::STRING: {
        PROTECT(res = NEW_CHARACTER(1));
        SET_STRING_ELT(res, 0, mkChar(vleval->toString().value().c_str()));
        // Note String/character is not given a class name because not
        // ambiguous  handle by implicit conversion
        UNPROTECT(1);
        break;
    }
    case value::Value::TUPLE: {
        const value::Tuple& tupleVal = vleval->toTuple();
        PROTECT(res = NEW_NUMERIC(tupleVal.size()));
        value::Tuple::const_iterator itb = tupleVal.value().begin();
        value::Tuple::const_iterator ite = tupleVal.value().end();
        for (int i = 0; itb != ite; ++i, itb++) {
            REAL(res)[i] = *itb;
        }
        if (without_class_names == 0) {
            SET_CLASS(res, mkString("VleTUPLE"));
        }
        UNPROTECT(1); // res
        break;
    }
    case value::Value::SET: {
        value::Set& setVal = vleval->toSet();
        if (multiple_values && unlist_multiple_values && setVal.size() == 1) {
            res = rvle_toRvalue(setVal.get(0).get(),
                                without_class_names,
                                0,
                                0,
                                matrix_type,
                                matrix_type_depth - 1);
        } else {
            PROTECT(res = allocVector(VECSXP, setVal.size()));
            value::Set::iterator itb = setVal.begin();
            value::Set::iterator ite = setVal.end();
            for (int i = 0; itb != ite; ++i, itb++) {
                SET_VECTOR_ELT(res,
                               i,
                               rvle_toRvalue(itb->get(),
                                             without_class_names,
                                             0,
                                             0,
                                             matrix_type,
                                             matrix_type_depth - 1));
            }
            if (without_class_names == 0) {
                if (multiple_values) {
                    SET_CLASS(res, mkString("VleMULTIPLE_VALUES"));
                } else {
                    SET_CLASS(res, mkString("VleSET"));
                }
            }
            UNPROTECT(1); // res
        }
        break;
    }
    case value::Value::MAP: {
        value::Map& mapVal = vleval->toMap();
        PROTECT(res = allocVector(VECSXP, mapVal.size()));
        SEXP resNames;
        PROTECT(resNames = NEW_CHARACTER(mapVal.size()));
        value::Map::iterator itb = mapVal.begin();
        value::Map::iterator ite = mapVal.end();
        // build values
        for (int i = 0; itb != ite; ++i, itb++) {
            SET_VECTOR_ELT(res,
                           i,
                           rvle_toRvalue(itb->second.get(),
                                         without_class_names,
                                         0,
                                         0,
                                         matrix_type,
                                         matrix_type_depth - 1));
        }
        // build names
        itb = mapVal.begin();
        for (int i = 0; itb != ite; ++i, itb++) {
            SET_STRING_ELT(resNames, i, mkChar(itb->first.c_str()));
        }
        SET_NAMES(res, resNames);
        UNPROTECT(1); // resNames
        if (without_class_names == 0) {
            SET_CLASS(res, mkString("VleMAP"));
        }
        UNPROTECT(1); // res
        break;
    }
    case value::Value::TABLE: {
        value::Table& tableVal = vleval->toTable();
        int height = static_cast<int>(tableVal.height());
        int width = static_cast<int>(tableVal.width());

        PROTECT(res = allocMatrix(REALSXP, height, width));
        for (int i = 0; i < width; ++i)
            for (int j = 0; j < height; ++j)
                REAL(res)[j + i * height] = tableVal.get(i, j);

        if (without_class_names == 0)
            SET_CLASS(res, mkString("VleTABLE"));

        UNPROTECT(1); // res
        break;
    }
    case value::Value::XMLTYPE: {
        PROTECT(res = NEW_CHARACTER(1));
        SET_STRING_ELT(
          res, 0, mkChar(vleval->toXml().writeToString().c_str()));
        if (without_class_names == 0) {
            SET_CLASS(res, mkString("VleXMLTYPE"));
        }
        UNPROTECT(1); // res
        break;
    }
    case value::Value::NIL: {
        PROTECT(res = allocVector(REALSXP, 1));
        REAL(res)[0] = NA_REAL; // WARNING NILSXP can lead to seg faults
        if (without_class_names == 0) {
            SET_CLASS(res, mkString("VleNIL"));
        }
        UNPROTECT(1); // res
        break;
    }
    case value::Value::MATRIX: {
        value::Matrix& matrixVal = vleval->toMatrix();
        if (matrix_type == 0 || matrix_type_depth > 0) {
            int rows = static_cast<int>(matrixVal.rows());
            int columns = static_cast<int>(matrixVal.columns());

            // list with attribute 'dim'
            PROTECT(res = allocMatrix(VECSXP, rows, columns));
            for (int i = 0; i < columns; ++i)
                for (int j = 0; j < rows; ++j)
                    SET_VECTOR_ELT(res,
                                   (j + i * rows),
                                   rvle_toRvalue(matrixVal.get(i, j).get(),
                                                 without_class_names,
                                                 0,
                                                 0,
                                                 matrix_type,
                                                 matrix_type_depth - 1));

            if (without_class_names == 0)
                SET_CLASS(res, mkString("VleMATRIX"));

            UNPROTECT(1); // res
        } else if (matrix_type == 1) {
            // dataframe with names into first rows
            SEXP names, value;
            std::string colName;
            PROTECT(res = NEW_LIST(matrixVal.columns()));
            PROTECT(names = NEW_CHARACTER(matrixVal.columns()));
            for (unsigned int col = 0; col < matrixVal.columns(); col++) {
                colName = matrixVal.get(col, 0)->toString().value();
                SET_STRING_ELT(names, col, mkChar(colName.c_str()));
                value::Value::type colType;
                if (matrixVal.rows() > 1) {
                    colType = rvle_get_vector_type(matrixVal, col, true);
                    switch (colType) {
                    case value::Value::BOOLEAN: {
                        PROTECT(value = NEW_LOGICAL(matrixVal.rows() - 1));
                        for (unsigned int r = 1; r < matrixVal.rows(); r++) {
                            const std::unique_ptr<value::Value>& v =
                              matrixVal.get(col, r);
                            if (v and
                                (v->getType() == value::Value::BOOLEAN)) {
                                LOGICAL(value)[r - 1] = v->toBoolean().value();
                            } else {
                                LOGICAL(value)[r - 1] = NA_LOGICAL;
                            }
                        }
                        UNPROTECT(1);
                        break;
                    }
                    case value::Value::DOUBLE: {

                        PROTECT(value = NEW_NUMERIC(matrixVal.rows() - 1));
                        for (unsigned int r = 1; r < matrixVal.rows(); r++) {
                            const std::unique_ptr<value::Value>& v =
                              matrixVal.get(col, r);
                            if (v and (v->getType() == value::Value::DOUBLE)) {
                                REAL(value)[r - 1] = v->toDouble().value();
                            } else {
                                REAL(value)[r - 1] = NA_REAL;
                            }
                        }
                        UNPROTECT(1);
                        break;
                    }
                    case value::Value::INTEGER: {
                        PROTECT(value = NEW_INTEGER(matrixVal.rows() - 1));
                        for (unsigned int r = 1; r < matrixVal.rows(); r++) {
                            const std::unique_ptr<value::Value>& v =
                              matrixVal.get(col, r);
                            if (v and
                                (v->getType() == value::Value::INTEGER)) {
                                INTEGER(value)[r - 1] = v->toInteger().value();
                            } else {
                                INTEGER(value)[r - 1] = NA_INTEGER;
                            }
                        }
                        UNPROTECT(1);
                        break;
                    }
                    case value::Value::STRING: {
                        PROTECT(value = NEW_CHARACTER(matrixVal.rows() - 1));
                        for (unsigned int r = 1; r < matrixVal.rows(); r++) {
                            const std::unique_ptr<value::Value>& v =
                              matrixVal.get(col, r);
                            if (v and (v->getType() == value::Value::STRING)) {
                                SET_STRING_ELT(
                                  value,
                                  r - 1,
                                  mkChar(v->toString().value().c_str()));
                            } else {
                                SET_STRING_ELT(value, r - 1, NA_STRING);
                            }
                        }
                        UNPROTECT(1);
                        break;
                    }
                    default: {
                        PROTECT(value = NEW_LIST(matrixVal.rows() - 1));
                        for (unsigned int r = 1; r < matrixVal.rows(); r++) {
                            const std::unique_ptr<value::Value>& v =
                              matrixVal.get(col, r);
                            if (v) {
                                SET_VECTOR_ELT(
                                  value,
                                  r - 1,
                                  rvle_toRvalue(v.get(),
                                                1 /*without_class_names*/,
                                                0 /*multiple_values*/,
                                                0 /*unlist_multiple_values*/,
                                                0 /*matrix_type*/,
                                                0 /*matrix_type_depth*/));
                            }
                        }
                        UNPROTECT(1);
                        break;
                    }
                    }
                    SET_VECTOR_ELT(res, col, value);
                }
            }
            /* set the first column name's */
            PROTECT(value = NEW_CHARACTER(matrixVal.rows() - 1));
            for (std::size_t i = 0; i < matrixVal.rows() - 1; ++i) {
                SET_STRING_ELT(value, i, mkChar(std::to_string(i).c_str()));
            }
            setAttrib(res, R_RowNamesSymbol, value);
            UNPROTECT(1); // value with row names
            SET_NAMES(res, names);
            UNPROTECT(1); // names
            SET_CLASS(res, mkString("data.frame"));
            UNPROTECT(1); // res
        } else if (matrix_type == 2) {
            // matrix with names into first rows
            int nbcol = static_cast<int>(matrixVal.columns());
            int nbrow = static_cast<int>(matrixVal.rows());

            PROTECT(res = allocMatrix(REALSXP, nbrow, nbcol));
            for (int i = 0; i < nbcol; ++i) {
                for (int j = 0; j < nbrow; ++j) {
                    const std::unique_ptr<value::Value>& v =
                      matrixVal.get(i, j);
                    if (v) {
                        switch (v->getType()) {
                        case value::Value::BOOLEAN:
                            REAL(res)
                            [j + i * nbrow] = (double)v->toBoolean().value();
                            break;
                        case value::Value::DOUBLE:
                            REAL(res)[j + i * nbrow] = v->toDouble().value();
                            break;
                        case value::Value::INTEGER:
                            REAL(res)
                            [j + i * nbrow] = (double)v->toInteger().value();
                            break;
                        default:
                            REAL(res)[j + i * nbrow] = NA_REAL;
                            break;
                        }
                    } else {
                        REAL(res)[j + i * nbrow] = NA_REAL;
                    }
                }
            }
            UNPROTECT(1); // res
        }
        break;
    }
    default: {
        error("not suppored type for (%i)", vleval->getType());
        break;
    }
    } // switch

    return res;
}

rvle_value_t
rvle_toVleValue(SEXP rval)
{

    value::Value* res = 0;
    SEXP rvalClassSexp = GET_CLASS(rval);

    if (rvalClassSexp == R_NilValue) {
        res = rvle_toVleValue_implicit(rval);
    } else {
        std::string rvalClass(CHAR(STRING_ELT(rvalClassSexp, 0)));
        if (rvalClass == "VleBOOLEAN") {
            res = new value::Boolean((bool)LOGICAL(rval)[0]);
        } else if (rvalClass == "VleDOUBLE") {
            if (TYPEOF(rval) == INTSXP) {
                res = new value::Double((double)INTEGER(rval)[0]);
            } else if (TYPEOF(rval) == REALSXP) {
                res = new value::Double((double)REAL(rval)[0]);
            }
        } else if (rvalClass == "VleINTEGER") {
            if (TYPEOF(rval) == INTSXP) {
                res = new value::Integer((int)INTEGER(rval)[0]);
            } else if (TYPEOF(rval) == REALSXP) {
                res = new value::Integer((int)REAL(rval)[0]);
            }
        } else if (rvalClass == "VleSTRING") {
            res = new value::String(CHAR(STRING_ELT(rval, 0)));
        } else if (rvalClass == "VleTUPLE") {
            res = new value::Tuple();
            SEXP temp;
            PROTECT(temp = AS_NUMERIC(rval));
            for (R_len_t i = 0; i < length(temp); i++) {
                res->toTuple().add(REAL(temp)[i]);
            }
            UNPROTECT(1);
        } else if (rvalClass == "VleSET") {
            res = new value::Set();
            value::Value* valuei;
            SEXP temp;
            PROTECT(temp = AS_LIST(rval));
            for (R_len_t i = 0; i < length(temp); i++) {
                valuei = reinterpret_cast<value::Value*>(
                  rvle_toVleValue(VECTOR_ELT(temp, i)));
                res->toSet().add(std::unique_ptr<value::Value>(valuei));
            }
            UNPROTECT(1);
        } else if (rvalClass == "VleMULTIPLE_VALUES") {
            res = new value::Set();
            // to differentiate with set, a tag is added
            res->toSet().addString("__intern_rvle_multiplevalues__");
            value::Value* valuei;
            SEXP temp;
            PROTECT(temp = AS_LIST(rval));
            for (R_len_t i = 0; i < length(temp); i++) {
                valuei = reinterpret_cast<value::Value*>(
                  rvle_toVleValue(VECTOR_ELT(temp, i)));
                res->toSet().add(std::unique_ptr<value::Value>(valuei));
            }
            UNPROTECT(1);
        } else if (rvalClass == "VleMAP") {
            res = new value::Map();
            std::string namei;
            value::Value* valuei;
            SEXP temp;
            PROTECT(temp = AS_LIST(rval));
            SEXP names = GET_NAMES(temp);
            for (R_len_t i = 0; i < length(names); i++) {
                namei.assign(CHAR(STRING_ELT(names, i)));
                valuei = reinterpret_cast<value::Value*>(
                  rvle_toVleValue(VECTOR_ELT(temp, i)));
                res->toMap().add(namei, std::unique_ptr<value::Value>(valuei));
            }
            UNPROTECT(1);
        } else if (rvalClass == "VleTABLE") {
            res = new value::Table();
            SEXP temp;
            PROTECT(temp = AS_NUMERIC(rval));
            SEXP dimensions = GET_DIM(temp);
            int nbDims = GET_LENGTH(dimensions);
            if (nbDims != 2) {
                // TODO error
            }
            int* nbDimsInt = INTEGER_POINTER(dimensions);
            int nrow = nbDimsInt[0];
            int ncol = nbDimsInt[1];
            res->toTable().resize(ncol, nrow);
            for (int i = 0; i < ncol; i++) {
                for (int j = 0; j < nrow; j++) {
                    res->toTable().get(i, j) = REAL(temp)[j + i * nrow];
                }
            }
            UNPROTECT(1);
        } else if (rvalClass == "VleXMLTYPE") {
            res = new value::Xml(CHAR(STRING_ELT(rval, 0)));
        } else if (rvalClass == "VleNIL") {
            res = new value::Null();
        } else if (rvalClass == "VleMATRIX") {
            SEXP temp;
            PROTECT(temp = rval);
            SEXP dimensions = GET_DIM(temp);
            int nbDims = GET_LENGTH(dimensions);
            if (nbDims != 2) {
                // TODO error
            }
            int* nbDimsInt = INTEGER_POINTER(dimensions);
            int nrow = nbDimsInt[0];
            int ncol = nbDimsInt[1];
            res = new value::Matrix(
              ncol, nrow, 10, 10); // TODO, because of bug of resize
            for (int i = 0; i < ncol; i++) {
                for (int j = 0; j < nrow; j++) {
                    res->toMatrix().set(
                      i,
                      j,
                      std::unique_ptr<value::Value>(
                        reinterpret_cast<value::Value*>(
                          rvle_toVleValue(VECTOR_ELT(temp, j + i * nrow)))));
                }
            }
            UNPROTECT(1);
        } else {
            res = rvle_toVleValue_implicit(rval, rvalClass);
        }
    }
    if (res == 0) {
        res = new value::Set();
    }
    return res;
}
