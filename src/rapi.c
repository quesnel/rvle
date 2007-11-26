/** 
 * @file rapi.c
 * @brief 
 * @author The vle Development Team
 * @date 2007-11-14
 */

/*
 * Copyright (C) 2007 - The vle Development Team
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

/*
 *
 * forward declarations
 *
 */

static SEXP r_rvle_open(SEXP name);
static SEXP r_rvle_run(SEXP rvle);
static SEXP r_rvle_delete(SEXP rvle);
static SEXP r_rvle_condition_list(SEXP rvle);
static SEXP r_rvle_condition_size(SEXP rvle);
static SEXP r_rvle_condition_set_real(SEXP rvle, SEXP cnd, SEXP prt, SEXP val);
static SEXP r_rvle_condition_set_integer(SEXP rvle, SEXP cnd, SEXP prt, SEXP val);
static SEXP r_rvle_save(SEXP rvle, SEXP file);

/*
 *
 * R function registration
 *
 */

R_CallMethodDef callMethods[] =
{
        { "open", (DL_FUNC) r_rvle_open, 1},
        { "run", (DL_FUNC) r_rvle_run, 1},
        { "condition_list", (DL_FUNC) r_rvle_condition_list, 1},
        { "condition_size", (DL_FUNC) r_rvle_condition_size, 1},
        { "condition_set_real", (DL_FUNC) r_rvle_condition_set_real, 4},
        { "condition_set_integer", (DL_FUNC) r_rvle_condition_set_integer, 4},
        { "save", (DL_FUNC) r_rvle_save, 2},
        { NULL, NULL, 0}
};

void R_init_rvle(DllInfo* info)
{
        R_registerRoutines(info, NULL, callMethods, NULL, NULL);
}

void R_unload_rvle(DllInfo* info)
{
}

/*
 *
 * rvle
 *
 */

#include "rvle.h"

SEXP r_rvle_open(SEXP name)
{
        SEXP r = R_NilValue;

        void* p = (void*) rvle_open(CHAR(STRING_ELT(name, 0)));
        if (p) {
                PROTECT(r = R_MakeExternalPtr(p, R_NilValue, R_NilValue));
                R_RegisterCFinalizer(r, r_rvle_delete);
                UNPROTECT(1);
        }

        return r;
}

SEXP r_rvle_run(SEXP rvle)
{
        SEXP r;
        int result;

        PROTECT(r = allocVector(INTSXP, 1));
        result = rvle_run(R_ExternalPtrAddr(rvle));
        INTEGER(r)[0] = result;
        UNPROTECT(1);

        return r;
}

SEXP r_rvle_delete(SEXP rvle)
{
        SEXP r;
        int result;

        PROTECT(r = allocVector(INTSXP, 1));
        result = rvle_delete(R_ExternalPtrAddr(rvle));
        INTEGER(r)[0] = result;
        UNPROTECT(1);

        return r;
}

SEXP r_rvle_condition_list(SEXP rvle)
{
        SEXP r;         /* condition list result */
        SEXP current;   /* condition name */
        char** result;  /* string list from the vle api */
        int size;       /* size of the condition list from the vle api */
        int i;

        size = rvle_condition_size(R_ExternalPtrAddr(rvle));
        result = rvle_condition_list(R_ExternalPtrAddr(rvle));

        PROTECT(r = allocVector(STRSXP, size));
        for (i = 0; i < size; ++i) {
		SET_STRING_ELT(r, i, mkChar(result[i]));
        }

        for (i = 0; i < size; ++i) {
                free(result[i]);
        }
        free(result);
        UNPROTECT(1);

        return r;
}

SEXP r_rvle_condition_size(SEXP rvle)
{
        SEXP r;
        int result;

        PROTECT(r = allocVector(INTSXP, 1));
        result = rvle_condition_size(R_ExternalPtrAddr(rvle));
        INTEGER(r)[0] = result;
        UNPROTECT(1);

        return r;
}

SEXP r_rvle_condition_set_real(SEXP rvle, SEXP cnd, SEXP prt, SEXP val)
{
        SEXP r;
        int result;

        PROTECT(r = allocVector(INTSXP, 1));
        result = rvle_condition_set_real(R_ExternalPtrAddr(rvle),
                        CHAR(STRING_ELT(cnd, 0)),
                        CHAR(STRING_ELT(prt, 0)),
                        REAL(val)[0]);
        INTEGER(r)[0] = result;
        UNPROTECT(1);

        return r;
}

SEXP r_rvle_condition_set_integer(SEXP rvle, SEXP cnd, SEXP prt, SEXP val)
{
        SEXP r;
        int result;

        PROTECT(r = allocVector(INTSXP, 1));
        result = rvle_condition_set_integer(R_ExternalPtrAddr(rvle),
                        CHAR(STRING_ELT(cnd, 0)),
                        CHAR(STRING_ELT(prt, 0)),
                        INTEGER(val)[0]);
        INTEGER(r)[0] = result;
        UNPROTECT(1);

        return r;
}

SEXP r_rvle_save(SEXP rvle, SEXP file)
{
        SEXP r;
        int result;

        PROTECT(r = allocVector(INTSXP, 1));
        result = rvle_save(R_ExternalPtrAddr(rvle), CHAR(STRING_ELT(file, 0)));
        INTEGER(r)[0] = result;
        UNPROTECT(1);

        return r;
}
