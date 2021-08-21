/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** GUIX Component                                                        */
/**                                                                       */
/**   Utility (Utility)                                                   */
/**                                                                       */
/**************************************************************************/
#define GX_SOURCE_CODE


/* Include necessary system files.  */

#include "gx_api.h"
#include "gx_utility.h"

#if defined(GX_DYNAMIC_BIDI_TEXT_SUPPORT)
typedef struct GX_BIDI_MIRROR_STRUCT
{
    USHORT gx_bidi_code;
    USHORT gx_bidi_mirror_code;
} GX_BIDI_MIRROR;

static GX_CONST GX_BIDI_MIRROR _gx_bidi_mirroring[] =
{
    {0x0028, 0x0029}, /* LEFT PARENTHESIS */
    {0x0029, 0x0028}, /*RIGHT PARENTHESIS */
    {0x003C, 0x003E}, /*LESS-THAN SIGN */
    {0x003E, 0x003C}, /*GREATER-THAN SIGN */
    {0x005B, 0x005D}, /*LEFT SQUARE BRACKET */
    {0x005D, 0x005B}, /*RIGHT SQUARE BRACKET */
    {0x007B, 0x007D}, /*LEFT CURLY BRACKET */
    {0x007D, 0x007B}, /*RIGHT CURLY BRACKET */
    {0x00AB, 0x00BB}, /*LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
    {0x00BB, 0x00AB}, /*RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
    {0x0F3A, 0x0F3B}, /*TIBETAN MARK GUG RTAGS GYON */
    {0x0F3B, 0x0F3A}, /*TIBETAN MARK GUG RTAGS GYAS */
    {0x0F3C, 0x0F3D}, /*TIBETAN MARK ANG KHANG GYON */
    {0x0F3D, 0x0F3C}, /*TIBETAN MARK ANG KHANG GYAS */
    {0x169B, 0x169C}, /*OGHAM FEATHER MARK */
    {0x169C, 0x169B}, /*OGHAM REVERSED FEATHER MARK */
    {0x2039, 0x203A}, /*SINGLE LEFT-POINTING ANGLE QUOTATION MARK */
    {0x203A, 0x2039}, /*SINGLE RIGHT-POINTING ANGLE QUOTATION MARK */
    {0x2045, 0x2046}, /*LEFT SQUARE BRACKET WITH QUILL */
    {0x2046, 0x2045}, /*RIGHT SQUARE BRACKET WITH QUILL */
    {0x207D, 0x207E}, /*SUPERSCRIPT LEFT PARENTHESIS */
    {0x207E, 0x207D}, /*SUPERSCRIPT RIGHT PARENTHESIS */
    {0x208D, 0x208E}, /*SUBSCRIPT LEFT PARENTHESIS */
    {0x208E, 0x208D}, /*SUBSCRIPT RIGHT PARENTHESIS */
    {0x2208, 0x220B}, /*ELEMENT OF */
    {0x2209, 0x220C}, /*NOT AN ELEMENT OF */
    {0x220A, 0x220D}, /*SMALL ELEMENT OF */
    {0x220B, 0x2208}, /*CONTAINS AS MEMBER */
    {0x220C, 0x2209}, /*DOES NOT CONTAIN AS MEMBER */
    {0x220D, 0x220A}, /*SMALL CONTAINS AS MEMBER */
    {0x2215, 0x29F5}, /*DIVISION SLASH */
    {0x223C, 0x223D}, /*TILDE OPERATOR */
    {0x223D, 0x223C}, /*REVERSED TILDE */
    {0x2243, 0x22CD}, /*ASYMPTOTICALLY EQUAL TO */
    {0x2252, 0x2253}, /*APPROXIMATELY EQUAL TO OR THE IMAGE OF */
    {0x2253, 0x2252}, /*IMAGE OF OR APPROXIMATELY EQUAL TO */
    {0x2254, 0x2255}, /*COLON EQUALS */
    {0x2255, 0x2254}, /*EQUALS COLON */
    {0x2264, 0x2265}, /*LESS-THAN OR EQUAL TO */
    {0x2265, 0x2264}, /*GREATER-THAN OR EQUAL TO */
    {0x2266, 0x2267}, /*LESS-THAN OVER EQUAL TO */
    {0x2267, 0x2266}, /*GREATER-THAN OVER EQUAL TO */
    {0x2268, 0x2269}, /*[BEST FIT] LESS-THAN BUT NOT EQUAL TO */
    {0x2269, 0x2268}, /*[BEST FIT] GREATER-THAN BUT NOT EQUAL TO */
    {0x226A, 0x226B}, /*MUCH LESS-THAN */
    {0x226B, 0x226A}, /*MUCH GREATER-THAN */
    {0x226E, 0x226F}, /*[BEST FIT] NOT LESS-THAN */
    {0x226F, 0x226E}, /*[BEST FIT] NOT GREATER-THAN */
    {0x2270, 0x2271}, /*[BEST FIT] NEITHER LESS-THAN NOR EQUAL TO */
    {0x2271, 0x2270}, /*[BEST FIT] NEITHER GREATER-THAN NOR EQUAL TO */
    {0x2272, 0x2273}, /*[BEST FIT] LESS-THAN OR EQUIVALENT TO */
    {0x2273, 0x2272}, /*[BEST FIT] GREATER-THAN OR EQUIVALENT TO */
    {0x2274, 0x2275}, /*[BEST FIT] NEITHER LESS-THAN NOR EQUIVALENT TO */
    {0x2275, 0x2274}, /*[BEST FIT] NEITHER GREATER-THAN NOR EQUIVALENT TO */
    {0x2276, 0x2277}, /*LESS-THAN OR GREATER-THAN */
    {0x2277, 0x2276}, /*GREATER-THAN OR LESS-THAN */
    {0x2278, 0x2279}, /*[BEST FIT] NEITHER LESS-THAN NOR GREATER-THAN */
    {0x2279, 0x2278}, /*[BEST FIT] NEITHER GREATER-THAN NOR LESS-THAN */
    {0x227A, 0x227B}, /*PRECEDES */
    {0x227B, 0x227A}, /*SUCCEEDS */
    {0x227C, 0x227D}, /*PRECEDES OR EQUAL TO */
    {0x227D, 0x227C}, /*SUCCEEDS OR EQUAL TO */
    {0x227E, 0x227F}, /*[BEST FIT] PRECEDES OR EQUIVALENT TO */
    {0x227F, 0x227E}, /*[BEST FIT] SUCCEEDS OR EQUIVALENT TO */
    {0x2280, 0x2281}, /*[BEST FIT] DOES NOT PRECEDE */
    {0x2281, 0x2280}, /*[BEST FIT] DOES NOT SUCCEED */
    {0x2282, 0x2283}, /*SUBSET OF */
    {0x2283, 0x2282}, /*SUPERSET OF */
    {0x2284, 0x2285}, /*[BEST FIT] NOT A SUBSET OF */
    {0x2285, 0x2284}, /*[BEST FIT] NOT A SUPERSET OF */
    {0x2286, 0x2287}, /*SUBSET OF OR EQUAL TO */
    {0x2287, 0x2286}, /*SUPERSET OF OR EQUAL TO */
    {0x2288, 0x2289}, /*[BEST FIT] NEITHER A SUBSET OF NOR EQUAL TO */
    {0x2289, 0x2288}, /*[BEST FIT] NEITHER A SUPERSET OF NOR EQUAL TO */
    {0x228A, 0x228B}, /*[BEST FIT] SUBSET OF WITH NOT EQUAL TO */
    {0x228B, 0x228A}, /*[BEST FIT] SUPERSET OF WITH NOT EQUAL TO */
    {0x228F, 0x2290}, /*SQUARE IMAGE OF */
    {0x2290, 0x228F}, /*SQUARE ORIGINAL OF */
    {0x2291, 0x2292}, /*SQUARE IMAGE OF OR EQUAL TO */
    {0x2292, 0x2291}, /*SQUARE ORIGINAL OF OR EQUAL TO */
    {0x2298, 0x29B8}, /*CIRCLED DIVISION SLASH */
    {0x22A2, 0x22A3}, /*RIGHT TACK */
    {0x22A3, 0x22A2}, /*LEFT TACK */
    {0x22A6, 0x2ADE}, /*ASSERTION */
    {0x22A8, 0x2AE4}, /*TRUE */
    {0x22A9, 0x2AE3}, /*FORCES */
    {0x22AB, 0x2AE5}, /*DOUBLE VERTICAL BAR DOUBLE RIGHT TURNSTILE */
    {0x22B0, 0x22B1}, /*PRECEDES UNDER RELATION */
    {0x22B1, 0x22B0}, /*SUCCEEDS UNDER RELATION */
    {0x22B2, 0x22B3}, /*NORMAL SUBGROUP OF */
    {0x22B3, 0x22B2}, /*CONTAINS AS NORMAL SUBGROUP */
    {0x22B4, 0x22B5}, /*NORMAL SUBGROUP OF OR EQUAL TO */
    {0x22B5, 0x22B4}, /*CONTAINS AS NORMAL SUBGROUP OR EQUAL TO */
    {0x22B6, 0x22B7}, /*ORIGINAL OF */
    {0x22B7, 0x22B6}, /*IMAGE OF */
    {0x22C9, 0x22CA}, /*LEFT NORMAL FACTOR SEMIDIRECT PRODUCT */
    {0x22CA, 0x22C9}, /*RIGHT NORMAL FACTOR SEMIDIRECT PRODUCT */
    {0x22CB, 0x22CC}, /*LEFT SEMIDIRECT PRODUCT */
    {0x22CC, 0x22CB}, /*RIGHT SEMIDIRECT PRODUCT */
    {0x22CD, 0x2243}, /*REVERSED TILDE EQUALS */
    {0x22D0, 0x22D1}, /*DOUBLE SUBSET */
    {0x22D1, 0x22D0}, /*DOUBLE SUPERSET */
    {0x22D6, 0x22D7}, /*LESS-THAN WITH DOT */
    {0x22D7, 0x22D6}, /*GREATER-THAN WITH DOT */
    {0x22D8, 0x22D9}, /*VERY MUCH LESS-THAN */
    {0x22D9, 0x22D8}, /*VERY MUCH GREATER-THAN */
    {0x22DA, 0x22DB}, /*LESS-THAN EQUAL TO OR GREATER-THAN */
    {0x22DB, 0x22DA}, /*GREATER-THAN EQUAL TO OR LESS-THAN */
    {0x22DC, 0x22DD}, /*EQUAL TO OR LESS-THAN */
    {0x22DD, 0x22DC}, /*EQUAL TO OR GREATER-THAN */
    {0x22DE, 0x22DF}, /*EQUAL TO OR PRECEDES */
    {0x22DF, 0x22DE}, /*EQUAL TO OR SUCCEEDS */
    {0x22E0, 0x22E1}, /*[BEST FIT] DOES NOT PRECEDE OR EQUAL */
    {0x22E1, 0x22E0}, /*[BEST FIT] DOES NOT SUCCEED OR EQUAL */
    {0x22E2, 0x22E3}, /*[BEST FIT] NOT SQUARE IMAGE OF OR EQUAL TO */
    {0x22E3, 0x22E2}, /*[BEST FIT] NOT SQUARE ORIGINAL OF OR EQUAL TO */
    {0x22E4, 0x22E5}, /*[BEST FIT] SQUARE IMAGE OF OR NOT EQUAL TO */
    {0x22E5, 0x22E4}, /*[BEST FIT] SQUARE ORIGINAL OF OR NOT EQUAL TO */
    {0x22E6, 0x22E7}, /*[BEST FIT] LESS-THAN BUT NOT EQUIVALENT TO */
    {0x22E7, 0x22E6}, /*[BEST FIT] GREATER-THAN BUT NOT EQUIVALENT TO */
    {0x22E8, 0x22E9}, /*[BEST FIT] PRECEDES BUT NOT EQUIVALENT TO */
    {0x22E9, 0x22E8}, /*[BEST FIT] SUCCEEDS BUT NOT EQUIVALENT TO */
    {0x22EA, 0x22EB}, /*[BEST FIT] NOT NORMAL SUBGROUP OF */
    {0x22EB, 0x22EA}, /*[BEST FIT] DOES NOT CONTAIN AS NORMAL SUBGROUP */
    {0x22EC, 0x22ED}, /*[BEST FIT] NOT NORMAL SUBGROUP OF OR EQUAL TO */
    {0x22ED, 0x22EC}, /*[BEST FIT] DOES NOT CONTAIN AS NORMAL SUBGROUP OR EQUAL */
    {0x22F0, 0x22F1}, /*UP RIGHT DIAGONAL ELLIPSIS */
    {0x22F1, 0x22F0}, /*DOWN RIGHT DIAGONAL ELLIPSIS */
    {0x22F2, 0x22FA}, /*ELEMENT OF WITH LONG HORIZONTAL STROKE */
    {0x22F3, 0x22FB}, /*ELEMENT OF WITH VERTICAL BAR AT END OF HORIZONTAL STROKE */
    {0x22F4, 0x22FC}, /*SMALL ELEMENT OF WITH VERTICAL BAR AT END OF HORIZONTAL STROKE */
    {0x22F6, 0x22FD}, /*ELEMENT OF WITH OVERBAR */
    {0x22F7, 0x22FE}, /*SMALL ELEMENT OF WITH OVERBAR */
    {0x22FA, 0x22F2}, /*CONTAINS WITH LONG HORIZONTAL STROKE */
    {0x22FB, 0x22F3}, /*CONTAINS WITH VERTICAL BAR AT END OF HORIZONTAL STROKE */
    {0x22FC, 0x22F4}, /*SMALL CONTAINS WITH VERTICAL BAR AT END OF HORIZONTAL STROKE */
    {0x22FD, 0x22F6}, /*CONTAINS WITH OVERBAR */
    {0x22FE, 0x22F7}, /*SMALL CONTAINS WITH OVERBAR */
    {0x2308, 0x2309}, /*LEFT CEILING */
    {0x2309, 0x2308}, /*RIGHT CEILING */
    {0x230A, 0x230B}, /*LEFT FLOOR */
    {0x230B, 0x230A}, /*RIGHT FLOOR */
    {0x2329, 0x232A}, /*LEFT-POINTING ANGLE BRACKET */
    {0x232A, 0x2329}, /*RIGHT-POINTING ANGLE BRACKET */
    {0x2768, 0x2769}, /*MEDIUM LEFT PARENTHESIS ORNAMENT */
    {0x2769, 0x2768}, /*MEDIUM RIGHT PARENTHESIS ORNAMENT */
    {0x276A, 0x276B}, /*MEDIUM FLATTENED LEFT PARENTHESIS ORNAMENT */
    {0x276B, 0x276A}, /*MEDIUM FLATTENED RIGHT PARENTHESIS ORNAMENT */
    {0x276C, 0x276D}, /*MEDIUM LEFT-POINTING ANGLE BRACKET ORNAMENT */
    {0x276D, 0x276C}, /*MEDIUM RIGHT-POINTING ANGLE BRACKET ORNAMENT */
    {0x276E, 0x276F}, /*HEAVY LEFT-POINTING ANGLE QUOTATION MARK ORNAMENT */
    {0x276F, 0x276E}, /*HEAVY RIGHT-POINTING ANGLE QUOTATION MARK ORNAMENT */
    {0x2770, 0x2771}, /*HEAVY LEFT-POINTING ANGLE BRACKET ORNAMENT */
    {0x2771, 0x2770}, /*HEAVY RIGHT-POINTING ANGLE BRACKET ORNAMENT */
    {0x2772, 0x2773}, /*LIGHT LEFT TORTOISE SHELL BRACKET ORNAMENT */
    {0x2773, 0x2772}, /*LIGHT RIGHT TORTOISE SHELL BRACKET ORNAMENT */
    {0x2774, 0x2775}, /*MEDIUM LEFT CURLY BRACKET ORNAMENT */
    {0x2775, 0x2774}, /*MEDIUM RIGHT CURLY BRACKET ORNAMENT */
    {0x27C3, 0x27C4}, /*OPEN SUBSET */
    {0x27C4, 0x27C3}, /*OPEN SUPERSET */
    {0x27C5, 0x27C6}, /*LEFT S-SHAPED BAG DELIMITER */
    {0x27C6, 0x27C5}, /*RIGHT S-SHAPED BAG DELIMITER */
    {0x27C8, 0x27C9}, /*REVERSE SOLIDUS PRECEDING SUBSET */
    {0x27C9, 0x27C8}, /*SUPERSET PRECEDING SOLIDUS */
    {0x27CB, 0x27CD}, /*MATHEMATICAL RISING DIAGONAL */
    {0x27CD, 0x27CB}, /*MATHEMATICAL FALLING DIAGONAL */
    {0x27D5, 0x27D6}, /*LEFT OUTER JOIN */
    {0x27D6, 0x27D5}, /*RIGHT OUTER JOIN */
    {0x27DD, 0x27DE}, /*LONG RIGHT TACK */
    {0x27DE, 0x27DD}, /*LONG LEFT TACK */
    {0x27E2, 0x27E3}, /*WHITE CONCAVE-SIDED DIAMOND WITH LEFTWARDS TICK */
    {0x27E3, 0x27E2}, /*WHITE CONCAVE-SIDED DIAMOND WITH RIGHTWARDS TICK */
    {0x27E4, 0x27E5}, /*WHITE SQUARE WITH LEFTWARDS TICK */
    {0x27E5, 0x27E4}, /*WHITE SQUARE WITH RIGHTWARDS TICK */
    {0x27E6, 0x27E7}, /*MATHEMATICAL LEFT WHITE SQUARE BRACKET */
    {0x27E7, 0x27E6}, /*MATHEMATICAL RIGHT WHITE SQUARE BRACKET */
    {0x27E8, 0x27E9}, /*MATHEMATICAL LEFT ANGLE BRACKET */
    {0x27E9, 0x27E8}, /*MATHEMATICAL RIGHT ANGLE BRACKET */
    {0x27EA, 0x27EB}, /*MATHEMATICAL LEFT DOUBLE ANGLE BRACKET */
    {0x27EB, 0x27EA}, /*MATHEMATICAL RIGHT DOUBLE ANGLE BRACKET */
    {0x27EC, 0x27ED}, /*MATHEMATICAL LEFT WHITE TORTOISE SHELL BRACKET */
    {0x27ED, 0x27EC}, /*MATHEMATICAL RIGHT WHITE TORTOISE SHELL BRACKET */
    {0x27EE, 0x27EF}, /*MATHEMATICAL LEFT FLATTENED PARENTHESIS */
    {0x27EF, 0x27EE}, /*MATHEMATICAL RIGHT FLATTENED PARENTHESIS */
    {0x2983, 0x2984}, /*LEFT WHITE CURLY BRACKET */
    {0x2984, 0x2983}, /*RIGHT WHITE CURLY BRACKET */
    {0x2985, 0x2986}, /*LEFT WHITE PARENTHESIS */
    {0x2986, 0x2985}, /*RIGHT WHITE PARENTHESIS */
    {0x2987, 0x2988}, /*Z NOTATION LEFT IMAGE BRACKET */
    {0x2988, 0x2987}, /*Z NOTATION RIGHT IMAGE BRACKET */
    {0x2989, 0x298A}, /*Z NOTATION LEFT BINDING BRACKET */
    {0x298A, 0x2989}, /*Z NOTATION RIGHT BINDING BRACKET */
    {0x298B, 0x298C}, /*LEFT SQUARE BRACKET WITH UNDERBAR */
    {0x298C, 0x298B}, /*RIGHT SQUARE BRACKET WITH UNDERBAR */
    {0x298D, 0x2990}, /*LEFT SQUARE BRACKET WITH TICK IN TOP CORNER */
    {0x298E, 0x298F}, /*RIGHT SQUARE BRACKET WITH TICK IN BOTTOM CORNER */
    {0x298F, 0x298E}, /*LEFT SQUARE BRACKET WITH TICK IN BOTTOM CORNER */
    {0x2990, 0x298D}, /*RIGHT SQUARE BRACKET WITH TICK IN TOP CORNER */
    {0x2991, 0x2992}, /*LEFT ANGLE BRACKET WITH DOT */
    {0x2992, 0x2991}, /*RIGHT ANGLE BRACKET WITH DOT */
    {0x2993, 0x2994}, /*LEFT ARC LESS-THAN BRACKET */
    {0x2994, 0x2993}, /*RIGHT ARC GREATER-THAN BRACKET */
    {0x2995, 0x2996}, /*DOUBLE LEFT ARC GREATER-THAN BRACKET */
    {0x2996, 0x2995}, /*DOUBLE RIGHT ARC LESS-THAN BRACKET */
    {0x2997, 0x2998}, /*LEFT BLACK TORTOISE SHELL BRACKET */
    {0x2998, 0x2997}, /*RIGHT BLACK TORTOISE SHELL BRACKET */
    {0x29B8, 0x2298}, /*CIRCLED REVERSE SOLIDUS */
    {0x29C0, 0x29C1}, /*CIRCLED LESS-THAN */
    {0x29C1, 0x29C0}, /*CIRCLED GREATER-THAN */
    {0x29C4, 0x29C5}, /*SQUARED RISING DIAGONAL SLASH */
    {0x29C5, 0x29C4}, /*SQUARED FALLING DIAGONAL SLASH */
    {0x29CF, 0x29D0}, /*LEFT TRIANGLE BESIDE VERTICAL BAR */
    {0x29D0, 0x29CF}, /*VERTICAL BAR BESIDE RIGHT TRIANGLE */
    {0x29D1, 0x29D2}, /*BOWTIE WITH LEFT HALF BLACK */
    {0x29D2, 0x29D1}, /*BOWTIE WITH RIGHT HALF BLACK */
    {0x29D4, 0x29D5}, /*TIMES WITH LEFT HALF BLACK */
    {0x29D5, 0x29D4}, /*TIMES WITH RIGHT HALF BLACK */
    {0x29D8, 0x29D9}, /*LEFT WIGGLY FENCE */
    {0x29D9, 0x29D8}, /*RIGHT WIGGLY FENCE */
    {0x29DA, 0x29DB}, /*LEFT DOUBLE WIGGLY FENCE */
    {0x29DB, 0x29DA}, /*RIGHT DOUBLE WIGGLY FENCE */
    {0x29F5, 0x2215}, /*REVERSE SOLIDUS OPERATOR */
    {0x29F8, 0x29F9}, /*BIG SOLIDUS */
    {0x29F9, 0x29F8}, /*BIG REVERSE SOLIDUS */
    {0x29FC, 0x29FD}, /*LEFT-POINTING CURVED ANGLE BRACKET */
    {0x29FD, 0x29FC}, /*RIGHT-POINTING CURVED ANGLE BRACKET */
    {0x2A2B, 0x2A2C}, /*MINUS SIGN WITH FALLING DOTS */
    {0x2A2C, 0x2A2B}, /*MINUS SIGN WITH RISING DOTS */
    {0x2A2D, 0x2A2E}, /*PLUS SIGN IN LEFT HALF CIRCLE */
    {0x2A2E, 0x2A2D}, /*PLUS SIGN IN RIGHT HALF CIRCLE */
    {0x2A34, 0x2A35}, /*MULTIPLICATION SIGN IN LEFT HALF CIRCLE */
    {0x2A35, 0x2A34}, /*MULTIPLICATION SIGN IN RIGHT HALF CIRCLE */
    {0x2A3C, 0x2A3D}, /*INTERIOR PRODUCT */
    {0x2A3D, 0x2A3C}, /*RIGHTHAND INTERIOR PRODUCT */
    {0x2A64, 0x2A65}, /*Z NOTATION DOMAIN ANTIRESTRICTION */
    {0x2A65, 0x2A64}, /*Z NOTATION RANGE ANTIRESTRICTION */
    {0x2A79, 0x2A7A}, /*LESS-THAN WITH CIRCLE INSIDE */
    {0x2A7A, 0x2A79}, /*GREATER-THAN WITH CIRCLE INSIDE */
    {0x2A7D, 0x2A7E}, /*LESS-THAN OR SLANTED EQUAL TO */
    {0x2A7E, 0x2A7D}, /*GREATER-THAN OR SLANTED EQUAL TO */
    {0x2A7F, 0x2A80}, /*LESS-THAN OR SLANTED EQUAL TO WITH DOT INSIDE */
    {0x2A80, 0x2A7F}, /*GREATER-THAN OR SLANTED EQUAL TO WITH DOT INSIDE */
    {0x2A81, 0x2A82}, /*LESS-THAN OR SLANTED EQUAL TO WITH DOT ABOVE */
    {0x2A82, 0x2A81}, /*GREATER-THAN OR SLANTED EQUAL TO WITH DOT ABOVE */
    {0x2A83, 0x2A84}, /*LESS-THAN OR SLANTED EQUAL TO WITH DOT ABOVE RIGHT */
    {0x2A84, 0x2A83}, /*GREATER-THAN OR SLANTED EQUAL TO WITH DOT ABOVE LEFT */
    {0x2A8B, 0x2A8C}, /*LESS-THAN ABOVE DOUBLE-LINE EQUAL ABOVE GREATER-THAN */
    {0x2A8C, 0x2A8B}, /*GREATER-THAN ABOVE DOUBLE-LINE EQUAL ABOVE LESS-THAN */
    {0x2A91, 0x2A92}, /*LESS-THAN ABOVE GREATER-THAN ABOVE DOUBLE-LINE EQUAL */
    {0x2A92, 0x2A91}, /*GREATER-THAN ABOVE LESS-THAN ABOVE DOUBLE-LINE EQUAL */
    {0x2A93, 0x2A94}, /*LESS-THAN ABOVE SLANTED EQUAL ABOVE GREATER-THAN ABOVE SLANTED EQUAL */
    {0x2A94, 0x2A93}, /*GREATER-THAN ABOVE SLANTED EQUAL ABOVE LESS-THAN ABOVE SLANTED EQUAL */
    {0x2A95, 0x2A96}, /*SLANTED EQUAL TO OR LESS-THAN */
    {0x2A96, 0x2A95}, /*SLANTED EQUAL TO OR GREATER-THAN */
    {0x2A97, 0x2A98}, /*SLANTED EQUAL TO OR LESS-THAN WITH DOT INSIDE */
    {0x2A98, 0x2A97}, /*SLANTED EQUAL TO OR GREATER-THAN WITH DOT INSIDE */
    {0x2A99, 0x2A9A}, /*DOUBLE-LINE EQUAL TO OR LESS-THAN */
    {0x2A9A, 0x2A99}, /*DOUBLE-LINE EQUAL TO OR GREATER-THAN */
    {0x2A9B, 0x2A9C}, /*DOUBLE-LINE SLANTED EQUAL TO OR LESS-THAN */
    {0x2A9C, 0x2A9B}, /*DOUBLE-LINE SLANTED EQUAL TO OR GREATER-THAN */
    {0x2AA1, 0x2AA2}, /*DOUBLE NESTED LESS-THAN */
    {0x2AA2, 0x2AA1}, /*DOUBLE NESTED GREATER-THAN */
    {0x2AA6, 0x2AA7}, /*LESS-THAN CLOSED BY CURVE */
    {0x2AA7, 0x2AA6}, /*GREATER-THAN CLOSED BY CURVE */
    {0x2AA8, 0x2AA9}, /*LESS-THAN CLOSED BY CURVE ABOVE SLANTED EQUAL */
    {0x2AA9, 0x2AA8}, /*GREATER-THAN CLOSED BY CURVE ABOVE SLANTED EQUAL */
    {0x2AAA, 0x2AAB}, /*SMALLER THAN */
    {0x2AAB, 0x2AAA}, /*LARGER THAN */
    {0x2AAC, 0x2AAD}, /*SMALLER THAN OR EQUAL TO */
    {0x2AAD, 0x2AAC}, /*LARGER THAN OR EQUAL TO */
    {0x2AAF, 0x2AB0}, /*PRECEDES ABOVE SINGLE-LINE EQUALS SIGN */
    {0x2AB0, 0x2AAF}, /*SUCCEEDS ABOVE SINGLE-LINE EQUALS SIGN */
    {0x2AB3, 0x2AB4}, /*PRECEDES ABOVE EQUALS SIGN */
    {0x2AB4, 0x2AB3}, /*SUCCEEDS ABOVE EQUALS SIGN */
    {0x2ABB, 0x2ABC}, /*DOUBLE PRECEDES */
    {0x2ABC, 0x2ABB}, /*DOUBLE SUCCEEDS */
    {0x2ABD, 0x2ABE}, /*SUBSET WITH DOT */
    {0x2ABE, 0x2ABD}, /*SUPERSET WITH DOT */
    {0x2ABF, 0x2AC0}, /*SUBSET WITH PLUS SIGN BELOW */
    {0x2AC0, 0x2ABF}, /*SUPERSET WITH PLUS SIGN BELOW */
    {0x2AC1, 0x2AC2}, /*SUBSET WITH MULTIPLICATION SIGN BELOW */
    {0x2AC2, 0x2AC1}, /*SUPERSET WITH MULTIPLICATION SIGN BELOW */
    {0x2AC3, 0x2AC4}, /*SUBSET OF OR EQUAL TO WITH DOT ABOVE */
    {0x2AC4, 0x2AC3}, /*SUPERSET OF OR EQUAL TO WITH DOT ABOVE */
    {0x2AC5, 0x2AC6}, /*SUBSET OF ABOVE EQUALS SIGN */
    {0x2AC6, 0x2AC5}, /*SUPERSET OF ABOVE EQUALS SIGN */
    {0x2ACD, 0x2ACE}, /*SQUARE LEFT OPEN BOX OPERATOR */
    {0x2ACE, 0x2ACD}, /*SQUARE RIGHT OPEN BOX OPERATOR */
    {0x2ACF, 0x2AD0}, /*CLOSED SUBSET */
    {0x2AD0, 0x2ACF}, /*CLOSED SUPERSET */
    {0x2AD1, 0x2AD2}, /*CLOSED SUBSET OR EQUAL TO */
    {0x2AD2, 0x2AD1}, /*CLOSED SUPERSET OR EQUAL TO */
    {0x2AD3, 0x2AD4}, /*SUBSET ABOVE SUPERSET */
    {0x2AD4, 0x2AD3}, /*SUPERSET ABOVE SUBSET */
    {0x2AD5, 0x2AD6}, /*SUBSET ABOVE SUBSET */
    {0x2AD6, 0x2AD5}, /*SUPERSET ABOVE SUPERSET */
    {0x2ADE, 0x22A6}, /*SHORT LEFT TACK */
    {0x2AE3, 0x22A9}, /*DOUBLE VERTICAL BAR LEFT TURNSTILE */
    {0x2AE4, 0x22A8}, /*VERTICAL BAR DOUBLE LEFT TURNSTILE */
    {0x2AE5, 0x22AB}, /*DOUBLE VERTICAL BAR DOUBLE LEFT TURNSTILE */
    {0x2AEC, 0x2AED}, /*DOUBLE STROKE NOT SIGN */
    {0x2AED, 0x2AEC}, /*REVERSED DOUBLE STROKE NOT SIGN */
    {0x2AF7, 0x2AF8}, /*TRIPLE NESTED LESS-THAN */
    {0x2AF8, 0x2AF7}, /*TRIPLE NESTED GREATER-THAN */
    {0x2AF9, 0x2AFA}, /*DOUBLE-LINE SLANTED LESS-THAN OR EQUAL TO */
    {0x2AFA, 0x2AF9}, /*DOUBLE-LINE SLANTED GREATER-THAN OR EQUAL TO */
    {0x2E02, 0x2E03}, /*LEFT SUBSTITUTION BRACKET */
    {0x2E03, 0x2E02}, /*RIGHT SUBSTITUTION BRACKET */
    {0x2E04, 0x2E05}, /*LEFT DOTTED SUBSTITUTION BRACKET */
    {0x2E05, 0x2E04}, /*RIGHT DOTTED SUBSTITUTION BRACKET */
    {0x2E09, 0x2E0A}, /*LEFT TRANSPOSITION BRACKET */
    {0x2E0A, 0x2E09}, /*RIGHT TRANSPOSITION BRACKET */
    {0x2E0C, 0x2E0D}, /*LEFT RAISED OMISSION BRACKET */
    {0x2E0D, 0x2E0C}, /*RIGHT RAISED OMISSION BRACKET */
    {0x2E1C, 0x2E1D}, /*LEFT LOW PARAPHRASE BRACKET */
    {0x2E1D, 0x2E1C}, /*RIGHT LOW PARAPHRASE BRACKET */
    {0x2E20, 0x2E21}, /*LEFT VERTICAL BAR WITH QUILL */
    {0x2E21, 0x2E20}, /*RIGHT VERTICAL BAR WITH QUILL */
    {0x2E22, 0x2E23}, /*TOP LEFT HALF BRACKET */
    {0x2E23, 0x2E22}, /*TOP RIGHT HALF BRACKET */
    {0x2E24, 0x2E25}, /*BOTTOM LEFT HALF BRACKET */
    {0x2E25, 0x2E24}, /*BOTTOM RIGHT HALF BRACKET */
    {0x2E26, 0x2E27}, /*LEFT SIDEWAYS U BRACKET */
    {0x2E27, 0x2E26}, /*RIGHT SIDEWAYS U BRACKET */
    {0x2E28, 0x2E29}, /*LEFT DOUBLE PARENTHESIS */
    {0x2E29, 0x2E28}, /*RIGHT DOUBLE PARENTHESIS */
    {0x3008, 0x3009}, /*LEFT ANGLE BRACKET */
    {0x3009, 0x3008}, /*RIGHT ANGLE BRACKET */
    {0x300A, 0x300B}, /*LEFT DOUBLE ANGLE BRACKET */
    {0x300B, 0x300A}, /*RIGHT DOUBLE ANGLE BRACKET */
    {0x300C, 0x300D}, /*[BEST FIT] LEFT CORNER BRACKET */
    {0x300D, 0x300C}, /*[BEST FIT] RIGHT CORNER BRACKET */
    {0x300E, 0x300F}, /*[BEST FIT] LEFT WHITE CORNER BRACKET */
    {0x300F, 0x300E}, /*[BEST FIT] RIGHT WHITE CORNER BRACKET */
    {0x3010, 0x3011}, /*LEFT BLACK LENTICULAR BRACKET */
    {0x3011, 0x3010}, /*RIGHT BLACK LENTICULAR BRACKET */
    {0x3014, 0x3015}, /*LEFT TORTOISE SHELL BRACKET */
    {0x3015, 0x3014}, /*RIGHT TORTOISE SHELL BRACKET */
    {0x3016, 0x3017}, /*LEFT WHITE LENTICULAR BRACKET */
    {0x3017, 0x3016}, /*RIGHT WHITE LENTICULAR BRACKET */
    {0x3018, 0x3019}, /*LEFT WHITE TORTOISE SHELL BRACKET */
    {0x3019, 0x3018}, /*RIGHT WHITE TORTOISE SHELL BRACKET */
    {0x301A, 0x301B}, /*LEFT WHITE SQUARE BRACKET */
    {0x301B, 0x301A}, /*RIGHT WHITE SQUARE BRACKET */
    {0xFE59, 0xFE5A}, /*SMALL LEFT PARENTHESIS */
    {0xFE5A, 0xFE59}, /*SMALL RIGHT PARENTHESIS */
    {0xFE5B, 0xFE5C}, /*SMALL LEFT CURLY BRACKET */
    {0xFE5C, 0xFE5B}, /*SMALL RIGHT CURLY BRACKET */
    {0xFE5D, 0xFE5E}, /*SMALL LEFT TORTOISE SHELL BRACKET */
    {0xFE5E, 0xFE5D}, /*SMALL RIGHT TORTOISE SHELL BRACKET */
    {0xFE64, 0xFE65}, /*SMALL LESS-THAN SIGN */
    {0xFE65, 0xFE64}, /*SMALL GREATER-THAN SIGN */
    {0xFF08, 0xFF09}, /*FULLWIDTH LEFT PARENTHESIS */
    {0xFF09, 0xFF08}, /*FULLWIDTH RIGHT PARENTHESIS */
    {0xFF1C, 0xFF1E}, /*FULLWIDTH LESS-THAN SIGN */
    {0xFF1E, 0xFF1C}, /*FULLWIDTH GREATER-THAN SIGN */
    {0xFF3B, 0xFF3D}, /*FULLWIDTH LEFT SQUARE BRACKET */
    {0xFF3D, 0xFF3B}, /*FULLWIDTH RIGHT SQUARE BRACKET */
    {0xFF5B, 0xFF5D}, /*FULLWIDTH LEFT CURLY BRACKET */
    {0xFF5D, 0xFF5B}, /*FULLWIDTH RIGHT CURLY BRACKET */
    {0xFF5F, 0xFF60}, /*FULLWIDTH LEFT WHITE PARENTHESIS */
    {0xFF60, 0xFF5F}, /*FULLWIDTH RIGHT WHITE PARENTHESIS */
    {0xFF62, 0xFF63}, /*[BEST FIT] HALFWIDTH LEFT CORNER BRACKET */
    {0xFF63, 0xFF62}, /*[BEST FIT] HALFWIDTH RIGHT CORNER BRACKET*/
    {0, 0}
};

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _gx_utility_bidi_mirroring_get                      PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Kenneth Maxwell, Microsoft Corporation                              */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the mirror code of a unicode.                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    code                                  Code point whose mirror code  */
/*                                            is retrieved                */
/*    mirror                                Retrieved mirror.             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Kenneth Maxwell          Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
UINT _gx_utility_bidi_mirroring_get(USHORT code, USHORT *mirror)
{
GX_CONST GX_BIDI_MIRROR *entry = _gx_bidi_mirroring;

    *mirror = 0;

    while (entry -> gx_bidi_code)
    {
        if (entry -> gx_bidi_code == code)
        {
            *mirror = entry -> gx_bidi_mirror_code;
            break;
        }
        entry++;
    }

    return GX_SUCCESS;
}

#endif

