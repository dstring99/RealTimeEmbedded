/*=============== G l o b a l s . h ===============*/

#ifndef GLOBALS_H
#define GLOBALS_H

//Task priorities
enum t_prio
{
	InitPrio = 1,		  /* -- Initialization task */
	InputPrio = 2,	  /* -- Input task */
	OutputPrio = 3,	  /* -- Output task */
        FramerPrio = 3,
        RobotMgrPrio = 4,
        RobotPrio = 4,
        ParserPrio = 4,
        RobotLocMgrPrio = 4,
};

//Type definition of Robot Position Struct
typedef struct
{
  CPU_INT08S x;
  CPU_INT08S y;
} Position;

//Sync byte definitions
#define SYNC_BYTE_0 0x03
#define SYNC_BYTE_1 0xAF
#define SYNC_BYTE_2 0xEF

//Command type defs
#define RS_TYPE 0x0
#define AD_TYPE 0x1
#define MV_TYPE 0x2
#define PA_TYPE 0x3
#define LP_TYPE 0x4
#define STOP_TYPE 0x5
#define STEP_TYPE 0x7
#define HERE_TYPE 0x9
#define ACK_TYPE 0xA
#define ERR_TYPE 0xB

//Length definitions
#define HEADER_LEN 4
#define ACK_LEN 5
#define RS_LEN 8
#define AD_LEN 11
#define MV_LEN 11
#define PA_LEN_MIN 11
#define PA_LEN_MAX 29
#define LP_LEN_MIN 11
#define LP_LEN_MAX 29
#define STOP_TYPE 0x5
#define STEP_LEN 5
#define MOVE_LEN 7
#define ERR_LEN 5

//Address definitions
#define CTRL_CENT_ADDR 1
#define RBT_MGR_ADDR 2
#define ADDR_MIN 3
#define ADDR_MAX 15

//Step Directions
#define DIR_0  0
#define DIR_N  1
#define DIR_NE 2
#define DIR_E  3
#define DIR_SE 4
#define DIR_S  5
#define DIR_SW 6
#define DIR_W  7
#define DIR_NW 8

//Step Direction Ofsets
#define OFF_DIR_0  {0,0}
#define OFF_DIR_N  {0,1}
#define OFF_DIR_NE {1,1}
#define OFF_DIR_E  {1,0}
#define OFF_DIR_SE {1,-1}
#define OFF_DIR_S  {0,-1}
#define OFF_DIR_SW {-1,-1}
#define OFF_DIR_W  {-1,0}
#define OFF_DIR_NW {-1,1}

//Err codes
#define ERR_CODE_CHK_SUM 4
#define ERR_CODE_PKT_LEN 5
#define ERR_AD_BAD_ADDR 11
#define ERR_AD_BAD_LOC 12
#define ERR_AD_LOC_OCC 13
#define ERR_AD_RBT_EXIST 14
#define ERR_MV_BAD_ADDR 21
#define ERR_MV_RBT_NONEXIST 22
#define ERR_MV_BAD_LOC 23
#define ERR_PA_BAD_ADDR 31
#define ERR_PA_RBT_NONEXIST 32
#define ERR_PA_BAD_LOC 33
#define ERR_LP_BAD_ADDR 41
#define ERR_LP_RBT_NONEXIST 42
#define ERR_LP_BAD_LOC 43
#define ERR_ST_BAD_ADDR 51
#define ERR_ST_RBT_NONEXIST 52
#define ERR_MSG_TYPE 61
#define ERR_RBT_GAVE_UP_03 103
#define ERR_RBT_GAVE_UP_04 104
#define ERR_RBT_GAVE_UP_05 105
#define ERR_RBT_GAVE_UP_06 106
#define ERR_RBT_GAVE_UP_07 107
#define ERR_RBT_GAVE_UP_08 108
#define ERR_RBT_GAVE_UP_09 109
#define ERR_RBT_GAVE_UP_10 110
#define ERR_RBT_GAVE_UP_11 111
#define ERR_RBT_GAVE_UP_12 112
#define ERR_RBT_GAVE_UP_13 113
#define ERR_RBT_GAVE_UP_14 114
#define ERR_RBT_GAVE_UP_15 115

//Coordinate limits of robot floor
#define POS_XMIN 0
#define POS_XMAX 39
#define POS_YMIN 0
#define POS_YMAX 18

#endif