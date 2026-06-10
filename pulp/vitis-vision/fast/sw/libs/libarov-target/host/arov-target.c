

/* =====================================================================
 * Project:      LibAROV
 * Title:        arov-target.c
 * Description:  Software APIs for accelerator-rich system.
 *
 * $Date:        13.7.2022
 * ===================================================================== */
/*
 * Copyright (C) 2022 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#include <arov-target.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The definition of AROV APIs is in the header file to force their inlining in the application executable.
// To control the AROV library generation, act on the generation/rendering parameters in the respective software libs generator.
// Read GenOv documentation for more infos.
