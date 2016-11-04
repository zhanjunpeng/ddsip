#ifndef CBDSIP_H
#define CBDSIP_H

#define DDSIP_SWAP(a,b) tmp=(a); (a)=(b); (b)=tmp;

#include <cplex.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#ifdef  CONIC_BUNDLE
#include <cb_cinterface.h>
#endif

#ifdef _WIN32
#include <time.h>
#pragma pack(push, 8)
#else
#include <sys/times.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
// Indicate the step in the b&b procedure
    enum DDSIP_step_t {
        dual, solve, neobj, eev, adv
    };

    typedef struct
    {

        // 1. CPLEX parameter handling
        // Priority order
        int    cpxorder;
        // Node limit
        int    cpxnodelim;
        // Node limit for upper bounds
        int    cpxubnodelim;
        // Output indicator
        int    cpxscr;
        // Output indicator for upper bounds
        int    cpxubscr;
        // Number of cplex parameters passed (default parameter set)
        int    cpxno;
        // Number of cplex parameters passed for lower bounds
        int    cpxnolb;
        // Number of cplex parameters passed for 2nd call of lower bounds
        int    cpxnolb2;
        // Number of cplex parameters passed for upper bounds
        int    cpxnoub;
        // Number of cplex parameters passed for 2nd call of upper bounds
        int    cpxnoub2;
        // Number of cplex parameters passed for ev problem
        int    cpxnoeev;
        // Number of cplex parameters passed for dual method
        int    cpxnodual;
        // Number of cplex parameters passed for 2nd call in dual method
        int    cpxnodual2;

        // Time limit
        double cpxtime;
        // Time limit for upper bounds
        double cpxubtime;
        // Relative gap
        double cpxgap;
        // Relative gap for upper bounds
        double cpxubgap;

        // For the different cplex parameter sets we have
        // which - the parameter number
        // isdbl - indicating whether the parameter is a double (=1) or integer (=0) parameter
        // what  - the parameter value
        int    *cpxwhich;
        int    *cpxlbwhich;
        int    *cpxlbwhich2;
        int    *cpxubwhich;
        int    *cpxubwhich2;
        int    *cpxeevwhich;
        int    *cpxdualwhich;
        int    *cpxdualwhich2;

        int    *cpxisdbl;
        int    *cpxlbisdbl;
        int    *cpxlbisdbl2;
        int    *cpxubisdbl;
        int    *cpxubisdbl2;
        int    *cpxeevisdbl;
        int    *cpxdualisdbl;
        int    *cpxdualisdbl2;

        double *cpxwhat;
        double *cpxlbwhat;
        double *cpxlbwhat2;
        double *cpxubwhat;
        double *cpxubwhat2;
        double *cpxeevwhat;
        double *cpxdualwhat;
        double *cpxdualwhat2;

        // Type of core file and output files
        char   *coretype;

        // 2. Conic Bundle
        // Use it in every cb-th iteration of the b+b procedure
        // if CONIC_BUNDLE is not defined, cb is set to 0 by the
        // specifications reader
        int   cb;
        // Representation of nonanticipativity
        int   nonant;
#ifdef CONIC_BUNDLE
        // (for cb < 0) number of nodes with cb iterations at beginning and before each 250th node
        int   cbContinuous;
        // (for cb < 0) number of nodes with cb iterations at beginning where a cb call is done only of more than half of the scenario solutions are inherited
        int   cbBreakIters;
        // Iteration limit (descent iterations)
        int   cbitlim;
        // Iteration limit in root node (descent iterations)
        int   cbrootitlim;
        // Iteration limit (total no. of iterations)
        int   cbtotalitlim;
        // Print level
        int   cbprint;
        // Bundle size
        int   cbbundlesz;
        // starting weight: in [0,+infty)
        double cbweight;
        // next weight = cbfactor*cbweight + (1-cbfactor)*(last weight in father): in [0,1]
        double cbfactor;
        // Relative precision
        double cbrelgap;
        // Maximal number of subgradients
        double cbmaxsubg;
        // inherit solutions in initial evaluation in CBLowerBound?
        int cb_inherit;
        // change relgap tolerance anf imelimit in CBLowerBound as long as bound is worde than in father node?
        int cb_changetol;
        // maximum number of CB inner iterarions in one descent step
        int cb_maxsteps;
        // reduce weight for CB if descent steps were successful within 3 total iters
        int cb_reduceWeight;
        // increase weight for CB if descent steps were unsuccessful or take too many iterations
        int cb_increaseWeight;
#endif

        // 3. Branch-and-bound
        // Output level
        int   outlev;
        // 'File level'
        int   files;
        //Read SMPS format
        // int   smps;
        // Start values (lower/upper bound, solution)
        int   advstart;
        // Warm starts
        int   hot;
        // Priority order
        int   order;
        // Pre- or Postfix of first-stage variables
        char * prefix;
        char * postfix;
/////// these are now determined from the model file plus pre-/postfix, thus transferred to data
        // Number of first-stage variables
        //int   firstvar;
        // Number of second-stage variables
        //int   secvar;
        // Number of first-stage constraints
        //int   firstcon;
        // Number of second-stage constraints
        //int   seccon;
///////////////////////////////////////////////////////////////////////////
        // Write deterministic equivalent? (only expectation-based so far)
        int   write_detequ;
        int   deteqType;
        // Number of variables for risk model
        int   riskvar;
        // Number of scenarios
        int   scenarios;
        // Number of stochastic right-hand sides
        int   stocrhs;
        // Number of stochastic matrix entries
        int   stocmat;
        // Number of stochastic cost coefficients
        int   stoccost;
        // Relaxation level for lower bounds
        // first-stage variables, second-stage variables, nonanticipativity
        int   relax;
        // Node limit
        int   nodelim;
        // Heuristic for upper bounds
        int   heuristic;
        int   heuristic_num;
        int   heuristic_auto;
        double* heuristic_vector;
        // Branching direction
        int   branchdir;
        // Branching strategy
        int   branchstrat;
        // Branch on integers first
        int   intfirst;
        // Branch on variables with dividing equally first
        int   equalbranch;
        // Bounding strategy
        int   boundstrat;
        // for boundstrat 10: frequency of using best bound
        int   bestboundfreq;
        // use premature stop in LB?
        int   prematureStop;
        // Update LP base in every i-th iteration
        int   base;
        // Log freqeuncy: Print output every i-ht iteration
        int   logfreq;
        // Solve EEV problem
        int   expected;
        // Sort scenarios
        int   prepro;
        // Calculate this number of quantils of the objective value distribution
        int   noquant;
        // gather kappa information (numerical problems?)
        int   watchkappa;

        // Time limit
        double timelim;
        // Relative gap
        double relgap;
        // Absolute gap
        double absgap;
        // Nulldispersion
        double nulldisp;
        // Accuracy
        double accuracy;
        // Branching on continuous variables uses this values
        double brancheps;

        // 4. Risk model
        // (Mean-)risk model
        int   riskmod;
// scalarization for mean-risk: 0=weighted sum, 1=reference point
        int   scalarization;
        // reference epsilon
        double ref_eps;
        // reference point and weights
        double * ref_point, * ref_scale;
        // Use fsd-consistency of risk measure
        int   riskalg;
        // Branch on additional risk variable eta
        int   brancheta;

        // Weight on risk term
        double riskweight;
        // Target (cost level)
        double risktarget;
        // Target (probability level)
        double risklevel;
        // Big M for risk model 2
        double riskM;

        // maximal number of levels of inheritance
        int maxinherit;
        // period for BOUSTRAT 1,2,3,4
        int period;
        // number of steps with small rgap for BOUSTRAT 1,2,3,4
        int rgapsmall;
    } para_t;

    typedef struct
    {

        // Number of variables in model
        int novar;
        // Number of constraints in model
        int nocon;
        // Number of first-stage variables
        int   firstvar;
        // Number of second-stage variables
        int   secvar;
        // Number of first-stage constraints
        int   firstcon;
        // Number of second-stage constraints
        int   seccon;
        // Stochastic right-hand sides
        double *rhs;
        // Probabilities
        double *prob;
        // Stochastic matrix entries
        double *matval;
        // Stochastic cost coefficients
        double *cost;

        // Row indices for stochastic rhs coefficients
        int    *rhsind;
        // Column indices for stochastic cost coefficients
        int    *costind;
        // Row indices for stochastic matrix entries
        int    *matrow;
        // Column indices for stochastic matrix entries
        int    *matcol;
        // Priority order
        int    *order;

        // Nonanticipativity matrix: cplex-like matrix
        int    *nabeg;
        int    *nacnt;
        int    *naind;
        double *naval;

    } data_t;

    typedef struct sug_l
    {
        double * firstval;
        struct sug_l *next;
    } sug_t;

    typedef struct
    {
        double * first_sol;
        double cursubsol, subbound;
    } bestfirst_t;

    typedef struct
    {

        // Number of first-stage variables
        int   firstvar;
        // Number of second-stage variables
        int   secvar;
        // Number of first-stage constraints
        int   firstcon;
        // Number of second-stage constraints
        int   seccon;
        // Number of variables
        int   novar;
        // Number of constraints
        int   nocon;

        // Number of violations of nonanticipativity in current node
        int    violations;
        // Skip evaluation of upper bound
        int    skip;
        // Number of bounds on variables at current node
        int    curbdcnt;
        // Total number of integers (for warm starts)
        int    total_int;
        // number of integers in first stage (including binaries)
        int    first_int;
        // number of binaries in first stage
        int    first_bin;
        // Depth of b+b-tree
        int    depth;
        // Number of nodes
        int    nonode;
        // Number of front nodes
        int    nofront;
        // Number of reduced front nodes (number of front nodes - number of leafs in front)
        int    no_reduced_front;
        // Index of current node
        int    curnode;
        // Number of evaluetad upper bounds
        int    neobjcnt;
        // Dimension of dual multipliers
        int    dimdual;
        // Indicator is 1 if a scenario has negative objective values
        int    isneg;
        // Number of dual iterations
        int    dualitcnt;
        // Number of decent dual iterations
        int    dualdescitcnt;
        // Index of lower bound constraint when using warm starts
        int    objbndind;

        // Counter of written core files
        int    lboutcnt;
        int    uboutcnt;

        // Step in b+b procedure
        enum DDSIP_step_t    DDSIP_step;

        // EEV solution
        double expbest;
        // Current lower bound (minimum of lower bounds of all front nodes)
        double bestbound;
        // Current best upper bound
        double bestvalue;
        // Lower bound in node which yielded best upper bound
        double feasbound;
        // Current upper bound
        double heurval;
        // Best value of risk measure
        double bestrisk;
        // Best value of expected value
        double bestexp;
        // Current best value of risk measure
        double currisk;
        // Current best value of expected value
        double curexp;

        // Risk-model: Absolute semideviation
        double target;

        // Value function
        /*    double *phiofTxph; */
        /*    double *Txph; */

        // Indices of first-stage variables
        int    *firstindex;
        int    *firstindex_reverse;
        // Indices of second-stage variables
        int    *secondindex;
        int    *secondindex_reverse;
        // Indices of first-stage constraints
        int    *firstrowind;
        int    *firstrowind_reverse;
        // Indices of second-stage constraints
        int    *secondrowind;
        int    *secondrowind_reverse;
        // Front of b+b-tree
        int    *front;
        // Indices of integer variables
        int    *intind;
        // Pointer to a vector of all integer variables in the solutions of the scenario problems
        // (for warm starts in CB)
        double **intsolvals;
        // Pointer to a vector of structures for the best found first stage variables in the solutions (in CB)
        bestfirst_t *bestfirst;
        // Priority order
        int    *order;
        // Solution status
        int    *solstat;
        // Indices of bounds on variables at current node
        int    *curind;

        // Vector of scenario problem solutions
        double *firstsol;
        // Best solution
        double *bestsol;
        // Original lower bounds
        double *lborg;
        // Original upper bounds
        double *uborg;
        // Contibution of individual variables to objective value
        double *objcontrib;
        // Best second-stage solution
        double **secstage;
        // Objective values of scenario problems for best upper bound
        double *subsol;
        // Lower bounds on variables at current node
        double *curlb;
        // Upper bounds on variables at current node
        double *curub;
        // Values for risk measures for best upper bound
        double *bestriskval;
        // Values for risk measures for current upper bound
        double *curriskval;
        //// Costs for dual iterations -> transferred to struct node
        //double *dual;
        // First-stage costs
        double *cost;
        // sorted list of front node indices
        int *front_nodes_sorted;

        // Heuristic solution in each node
        sug_t ** sug;

        // Wait-and-see lower bounds for calculation of 'best target'
        double *btlb;

        // C-Type of first-stage variables
        char   *firsttype;
        // C-Type of second-stage variables
        char   *sectype;
        // Lower bound identifier for cplex routine CPXchgbd
        char   *lbident;
        // Upper bound identifier for cplex routine CPXchgbd
        char   *ubident;
        //
        // reference point scalarization: max = exp (0) or = risk (1)
        int      ref_max;
        double*     ref_risk;
        // unbiased scenario objectives for the case of optimality in siplb
        double*  ref_scenobj;
        //
        // advanced start solution
        double* adv_sol;

        // pointer for re-sorting the scenarios for lower bounding
        int   *lb_scen_order;
        // pointer for re-sorting the scenarios for upper bounding
        int   *scen_order;

        // Text buffer for querying variable name
        char * name_buffer[1];
        char * n_buffer;
        int    n_buffer_len;

        // For wall time measurment
        time_t start_time, cur_time;

        // For use of MIP starts
        double * values;
        int    * effort;
        int    * beg;
        char  ** Names;
        // File name for additional output
        FILE *moreoutfile;
        int  noiter;
        double dualObjVal;
        int objIncrease;
        int heurSuccess;
        int cutoff;
        cb_problemp dualProblem;
        double correct_bounding;
        // set to 1 when the original is maximization
        int maximization;
    } bb_t;

    typedef struct
    {

        // The father node (-1 for root node)
        int    father;

        // The depth of the node (0 for root node)
        int    depth;

        // The step type in the node
        enum DDSIP_step_t    step;

        // The index to branch on
        // Will be calculated in Solve()
        int    branchind;

        // Has the node been solved already ?
        int    solved;

        // Is the node a leaf of the complete tree ?
        int    leaf;

        // The number of inherited scenario solutions
        int    numInheritedSols;

        // The neo-stuff specifies the new constraint in the node,
        // i.e. the variable neoind was bounded to neolb and neoub.
        int    neoind;
        double neolb;
        double neoub;

        // The dispersion norm of first stage vars according to the subsolutions in the
        // node - calculation in Solve()
        double dispnorm;

        // Number of violations of nonanticipativity in the node
        int    violations;

        // The value used for branching
        double branchval;

        // The lower bound in the node
        double bound;

        // For the risk measure absolute semideviation
        double target;

        // vector of vectors of first stage variable values of scenario solutions
        double **first_sol;
        // Integer objective values for the scenarios in this node
        double *cursubsol;
        // Bound for objective values for the scenarios in this node
        double *subbound;
        // objective values for the scenarios in this node without Lagrangean terms
        double *subboundNoLag;
        // mipstatus values for the scenarios in this node
        int *mipstatus;

        // reference objectives
        double *ref_scenobj;

        // The integer components of subsolutions in the node for warm start
        double *solut;

        // The bounds of the scenario problems
        double *scbound;

        // Cost coefficients when dual method is in use
        double *dual;

        // Best cost coefficients when dual method is in use
        double *bestdual;

    } node_t;

// DDSIP globals
// CPLEX environement and lp pointer
    extern CPXENVptr    DDSIP_env;
    extern CPXLPptr     DDSIP_lp;

    extern FILE         *DDSIP_outfile;


// These pointers have to be global if we want to use ConicBundle
    extern para_t          * DDSIP_param;
    extern data_t          * DDSIP_data;
    extern bb_t            * DDSIP_bb;
    extern node_t         ** DDSIP_node;

// Small functions
#ifdef _WIN32
    static _inline double DDSIP_Dmin(double a, double b)
    {
        return a<b ? a : b;
    }
    static _inline double DDSIP_Dmax(double a, double b)
    {
        return a>b ? a : b;
    }
    static _inline int DDSIP_Imin(int a, int b)
    {
        return a<b ? a : b;
    }
    static _inline int DDSIP_Imax(int a, int b)
    {
        return a>b ? a : b;
    }
// Accuracy comparison of two double numbers
    static _inline int DDSIP_Equal(double a, double b)
    {
        return (fabs(a-b)>DDSIP_param->accuracy * 0.5 * (fabs(a)+fabs(b))) ? 0 : 1;
    }
#else
    static inline double DDSIP_Dmin(double a, double b)
    {
        return a<b ? a : b;
    }
    static inline double DDSIP_Dmax(double a, double b)
    {
        return a>b ? a : b;
    }
    static inline int DDSIP_Imin(int a, int b)
    {
        return a<b ? a : b;
    }
    static inline int DDSIP_Imax(int a, int b)
    {
        return a>b ? a : b;
    }
// Accuracy comparison of two double numbers
    static inline int DDSIP_Equal(double a, double b)
    {
        return (fabs(a-b)> DDSIP_param->accuracy * 0.5 * (fabs(a)+fabs(b))) ? 0 : 1;
    }
#endif

    void DDSIP_qsort_ins_D(const double *, int *, int, int);
    void DDSIP_qsort_ins_A(const double *, int *, int, int);
    int  DDSIP_Error(int);
    int  DDSIP_NoSolution(int);
    int  DDSIP_Infeasible(int);
    void DDSIP_HandleSignal(int );
    void DDSIP_RegisterSignalHandlers(void);
    double DDSIP_GetCpuTime(void);
    void DDSIP_PrintErrorMsg(int);
    void DDSIP_Print2(char *, char *, double, int);
    void DDSIP_translate_time(double , int *, int *, double *);

// Reading
    int  DDSIP_ReadSpec(void);
    int  DDSIP_ReadCpxPara(FILE *);
    int  DDSIP_ReadModel(void);
    int  DDSIP_ReadCPLEXOrder(void);
    int  DDSIP_ReadData(void);

// Manage cplex parameter sets
    int  DDSIP_InitCpxPara(void);
    int  DDSIP_SetCpxPara(const int, const int*, const int*, const double*);
    int  DDSIP_CpxParaPrint(void);

// Initialisations and stuff
    void DDSIP_DetEqu(void);
    int  DDSIP_BbTypeInit(void);
    int  DDSIP_BranchOrder(void);
    int  DDSIP_InitStages(void);
    int  DDSIP_DetectStageRows(void);
    int  DDSIP_AdvSolInit(void);
    int  DDSIP_BbInit(void);
    int  DDSIP_AdvStart(void);

// EEV
    int DDSIP_ExpValProb(void);

// Risk model
    int  DDSIP_RiskModel(void);
    int  DDSIP_RiskObjective(double *);
    double DDSIP_RiskLb(double *);
    int  DDSIP_DeleteRiskObj(void);
    int  DDSIP_UndeleteRiskObj(void);
    int  DDSIP_SemDevChgBd(void);
    int  DDSIP_SemDevGetNodeTarget(void);
    int  DDSIP_BestTarget(void);

// B&B
    int  DDSIP_GetBranchIndex (double *);
    int  DDSIP_ChgBounds(void);
    int  DDSIP_ChgProb(int);
    int  DDSIP_LowerBound(void);
    int  DDSIP_Heuristics(int *);
    int  DDSIP_SolChk(void);
    int  DDSIP_UpperBound(void);
    int  DDSIP_RestoreBoundAndType(void);
    int  DDSIP_Bound(void);
    int  DDSIP_Branch(void);
    void DDSIP_PrintState(int);
    void DDSIP_PrintStateUB (int);
    int  DDSIP_Continue(int *, int *);

// Memory care
    void* DDSIP_Alloc(int,int,char *);
    void  DDSIP_Free(void **);
    void  DDSIP_FreeNode(int);
    void  DDSIP_FreeFrontNodes(void);
    void  DDSIP_FreeData(void);
    void  DDSIP_FreeBb(void);
    void  DDSIP_FreeParam(void);

// Lagrangian dual
    int DDSIP_NonAnt(void);
    int DDSIP_DualUpdate(void*, double *, double, int, double *, int*, double *, double *, double *);
    int DDSIP_DualOpt(void);
    int DDSIP_CBLowerBound(double *, double);
    int DDSIP_Contrib    (double *, int);
    int DDSIP_Contrib_LB (double *, int);

#ifdef __cplusplus
} // extern "C"
#endif
#ifdef _WIN32
#pragma pack(pop)
#endif
// further function declarations
#endif
