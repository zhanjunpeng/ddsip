/*  Authors:           Andreas M"arkert, Ralf Gollmer
    Copyright to:      University of Duisburg-Essen
   Language:          C
   Last modification: 06.02.2016
    Description:
    Functions to read a double number, the specification file, the model files,
    the scenario files and an advanced solution.

    License:
    This file is part of DDSIP.

    DDSIP is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    DDSIP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DDSIP; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include <DDSIP.h>
#include <DDSIPconst.h>
#include <math.h>

int DDSIP_SkipToEOL (FILE *);
int DDSIP_Find (FILE *, char *);
double DDSIP_ReadDbl (FILE *, char *, char *, double, int, double, double);
double *DDSIP_ReadDblVec (FILE *, char *, char *, double, int, double, double, int, int *);
char *DDSIP_ReadString (FILE *, char *, char *);
int DDSIP_ReadWord (FILE *, char *, int);

int
DDSIP_SkipToEOL (FILE * specfile)
{
    int c;
    while ((c = fgetc (specfile)) != EOF)
    {
        if (c == '\n')
            return (0);
    }
    return (EOF);
}

int
DDSIP_Find (FILE * specfile, char *pattern)
{
    int find = 0, i = 0;

    int c;
    char str[DDSIP_max_str_ln];

    // Rewind file
    rewind (specfile);
    // Read as long as str doesn't match pattern.
    // Only the first occurence of the pattern is relevant.
    while (!find && (c = fgetc (specfile)) != EOF)
    {
        if (isspace(c)||c=='*')
        {
            str[i] = '\0';
            if (!strncmp (str, pattern, DDSIP_Imin (DDSIP_unique, strlen (pattern))))
                find = 1;
            if (c=='*')
                DDSIP_SkipToEOL (specfile);
            i = 0;
        }
        else
            str[i++] = c;
    }

    return find;
}

//==========================================================================
// Function returns value that occurs in specfile right behind pattern
// It returns defval if there is no such pattern

double
DDSIP_ReadDbl (FILE * specfile, char *pattern, char *text, double defval, int isint, double lb, double ub)
{
    int find, i, ih;

    double val;

    int c;
    char str[DDSIP_max_str_ln];
    find = 0;
    i = 0;
    // Rewind file
    rewind (specfile);
    // Read as long as str doesn't match pattern.
    // Only the first occurence of the pattern is relevant.

    // Default ?
    val = defval;
    while (!find && (c = fgetc (specfile)) != EOF)
    {
        if (isspace (c))
        {
            str[i] = '\0';
            if (!strncmp (str, pattern, DDSIP_Imin (DDSIP_unique, strlen (pattern))))
            {
                find = 1;
                if (fscanf (specfile, "%lf", &val) != 1)
                {
                    printf ("*Error: could not read value for %s\n", pattern);
                }
            }
            i = 0;
        }
        else
        {
            if (c=='*')
            {
                DDSIP_SkipToEOL (specfile);
                i = 0;
            }
            else
                str[i++] = c;
        }
    }

    // Consistent value ?
    if (val < lb || val > ub)
    {
        find = 0;
        if (isint)
        {
            ih = floor (val + 0.1);
            printf ("*Warning: Illegal parameter setting: %s = %d.\n", text, ih);
            printf ("*         Reset parameter to %.0f (Parameter range: %.0f - %.0f).\n",
                    floor (defval + 0.1), floor (lb + 0.1), floor (ub + 0.1));
            val = floor (defval + 0.1);
        }
        else
        {
            printf ("*Warning: Illegal parameter setting (%s = %g).\n", text, val);
            printf ("*         Reset parameter to %g (Parameter range: %g - %g).\n", defval, lb, ub);
            val = defval;
        }
    }
    // Write info to output file
    if (!find)
    {
        // Two-stage parameters can be read from the time file, too. Some parameters are hidden.
        if (DDSIP_param->outlev
//      && strncmp (pattern, "FIRSTV", DDSIP_unique)
//      && strncmp (pattern, "FIRSTC", DDSIP_unique)
//      && strncmp (pattern, "SECVAR", DDSIP_unique)
//      && strncmp (pattern, "SECVAR", DDSIP_unique)
//      && strncmp (pattern, "ACCURA", DDSIP_unique)
           )
        {
            fprintf (DDSIP_outfile, " %-8s  %-31s(default) ", pattern, text);
            if (isint)
            {
                val = floor (val + 0.1);
                fprintf (DDSIP_outfile, "    %12.0f\n", val);
            }
            else
                fprintf (DDSIP_outfile, "    %12.3e\n", val);
        }
    }
    else
    {
        fprintf (DDSIP_outfile, " %-8s  %-40s ", pattern, text);
        if (isint)
        {
            val = floor (val + 0.1);
            fprintf (DDSIP_outfile, "    %12.0f\n", val);
        }
        else
            fprintf (DDSIP_outfile, "    %12.3e\n", val);
    }

    return val;
}

//==========================================================================
// Function returns vector of 10 values that occur in specfile right behind pattern
// It returns NULL if there is no such pattern

double *
DDSIP_ReadDblVec (FILE * specfile, char *pattern, char *text, double defval, int isint, double lb, double ub, int maxvals, int *number)
{
    int find, i, ih;

    double *val;

    int c;
    char str[DDSIP_max_str_ln];
    find = 0;
    *number = i = 0;
    // Rewind file
    rewind (specfile);
    // Read as long as str doesn't match pattern.
    // Only the first occurence of the pattern is relevant.

    // Default ?
    val = NULL;
    *number = 0;
    while (!find && (c = fgetc (specfile)) != EOF)
    {
        if (isspace (c))
        {
            str[i] = '\0';
            if (!strncmp (str, pattern, DDSIP_Imin (DDSIP_unique, strlen (pattern))))
            {
                find = 1;
                val = (double *) DDSIP_Alloc (sizeof (double), maxvals, "values(DDSIP_ReadDblVec)");
                for (ih = 0; ih < maxvals; ih++)
                {
                    if (fscanf (specfile, "%lf", val + ih) == 0)
                        break;
                    else
                    {
                        (*number)++;
                        // Consistent value ?
                        if (val[ih] < lb)
                        {
                            if (isint)
                            {
                                i = floor (val[ih] + 0.1);
                                printf ("*Warning: Illegal parameter setting: %s = %d.\n", text, i);
                                printf ("*         Reset parameter to %.0f (Parameter range: %.0f - %.0f).\n",
                                        floor (lb + 0.1), floor (lb + 0.1), floor (ub + 0.1));
                                val[ih] = floor (lb + 0.1);
                            }
                            else
                            {
                                printf ("*Warning: Illegal parameter setting (%s = %f).\n", text, val[ih]);
                                printf ("*         Reset parameter to %f (Parameter range: %f - %f).\n", lb, lb, ub);
                                val[ih] = lb;
                            }
                        }
                        else if (val[ih] > ub)
                        {
                            if (isint)
                            {
                                i = floor (val[ih] + 0.1);
                                printf ("*Warning: Illegal parameter setting: %s = %d.\n", text, i);
                                printf ("*         Reset parameter to %.0f (Parameter range: %.0f - %.0f).\n",
                                        floor (ub + 0.1), floor (lb + 0.1), floor (ub + 0.1));
                                val[ih] = floor (ub + 0.1);
                            }
                            else
                            {
                                printf ("*Warning: Illegal parameter setting (%s = %f).\n", text, val[ih]);
                                printf ("*         Reset parameter to %f (Parameter range: %f - %f).\n", ub, lb, ub);
                                val[ih] = ub;
                            }
                        }
                    }
                }
            }
            i = 0;
        }
        else
        {
            if (c=='*')
            {
                DDSIP_SkipToEOL (specfile);
                i = 0;
            }
            else
                str[i++] = c;
        }
    }

    // Write info to output file
    if (!find)
    {
        val = (double *) DDSIP_Alloc (sizeof (double), 1, "values(DDSIP_ReadDblVec)");
        val[0] = defval;
        *number = 1;
        fprintf (DDSIP_outfile, " %-8s  %-31s(default) ", pattern, text);
        if (isint)
        {
            val[0] = floor (val[0] + 0.1);
            fprintf (DDSIP_outfile, "    %12.0f\n", val[0]);
        }
        else
            fprintf (DDSIP_outfile, "    %12.3e\n", val[0]);
    }
    else
    {
        fprintf (DDSIP_outfile, " %-40s ", text);
        for (i = 0; i < *number; i++)
            if (isint)
            {
                val[i] = floor (val[i] + 0.1);
                fprintf (DDSIP_outfile, "    %12.0f\n", val[i]);
            }
            else
                fprintf (DDSIP_outfile, "    %12.3e\n", val[i]);
    }

    return val;
}

//==========================================================================
// Function returns pointer to string that occurs in specfile right behind pattern
// It returns NULL if there is no such pattern

char *
DDSIP_ReadString (FILE * specfile, char *pattern, char *text)
{
    int find, i;

    int c;
    char str[DDSIP_max_str_ln];
    char *string, *outstring=NULL;
    find = 0;
    i = 0;
    // Rewind file
    rewind (specfile);
    // Read as long as str doesn't match pattern.
    // Only the first occurence of the pattern is relevant.

    // Default ?
    string = NULL;
    while (!find && (c = fgetc (specfile)) != EOF)
    {
        if (isspace (c))
        {
            str[i] = '\0';
            if (!strncmp (str, pattern, DDSIP_Imin (DDSIP_unique, strlen (pattern))))
            {
                find = 1;
            }
            i = 0;
        }
        else
        {
            if (c=='*')
            {
                DDSIP_SkipToEOL (specfile);
                i = 0;
            }
            else
                str[i++] = c;
        }
    }
    if (find)
    {
        if (!(string = (char *) calloc (256, sizeof (char))))
        {
            printf ("XXX ERROR ReadString: could not allocate string of size 256.\n");
            exit (1);
        }
        while ((c = fgetc (specfile)) != EOF)
            if (!isspace (c))
                break;
        if (c == EOF)
        {
            printf ("XXX ERROR ReadString: found %s, but no following string.\n", pattern);
            exit (1);
        }
        string[0] = c;
        i = 1;
        while ((c = fgetc (specfile)) != EOF)
        {
            if (isspace (c))
                break;
            else
                string[i++] = c;
            if (i >= 255)
            {
                string[255] = '\0';
                printf ("XXX ERROR ReadString: string following %s is too long: %s\n", pattern, string);
                exit (1);
            }
        }
        
        fprintf (DDSIP_outfile, " %-8s  %-40s ", pattern, text);
        fprintf (DDSIP_outfile, "    %12s\n", string);
        if (!i)
            DDSIP_Free ((void **) &(string));
        else
        {
            string[i] = '\0';
            if (!(outstring = (char *) calloc (i+1, sizeof (char))))
            {
                printf ("XXX ERROR ReadString: could not allocate string of size %d.\n", i+1);
                exit (1);
            }
            strcpy(outstring, string);
            DDSIP_Free ((void **) &(string));
        }
    }
    return (outstring);
}

//==========================================================================
// Function reads the next word (a sequence of chars different from whitespace) from infile
// into the given string array. stringlength has to specify the number of chars string can hold (including the terminating \0)
// It returns the length of the string read (excluding the terminating \0)

int 
DDSIP_ReadWord (FILE * infile, char *string, int stringlength)
{
    int i;
    int c;

    string[0] = '\0';
    while ((c = fgetc (infile)) != EOF)
        if (!isspace (c))
            break;
    if (c == EOF)
    {
        printf ("XXX Warning ReadWord: no word found up to EOF.\n");
        return (0);
    }
    string[0] = c;
    i = 1;
    while ((c = fgetc (infile)) != EOF)
    {
        if (isspace (c))
            break;
        else
            string[i++] = c;
        if (i >= stringlength - 1)
        {
            string[stringlength -1] = '\0';
            printf ("XXX ERROR ReadWord: word is too long for array string of size %d: %s\n", stringlength, string);
            exit (1);
        }
    }
    string[i] = '\0';
//////////////// Debug
printf ("ReadWord: '%s'\n", string);
//////////////// Debug
    return (i);
}

//==========================================================================
int
DDSIP_ReadCpxPara (FILE * specfile)
{
    int find = 0, i = 0, cnt, k, j, status;

    int c;
    char str[DDSIP_max_str_ln];
    char pattern[DDSIP_max_str_ln];

    find = 0;
    sprintf (pattern, "CPLEXBEGIN");
    // Search for 'CPLEXBEGIN' from beginning of specfile
    rewind (specfile);
    while (!find && (c = fgetc (specfile)) != EOF)
        if (c == '\n' || c == ' ' || c == '*' )
        {
            str[i] = '\0';
            if (!strncmp (str, pattern, strlen (pattern)))
                find = 1;
            i = 0;
            // Read till end of line
            if (c != '\n')
                while ((c = fgetc (specfile)) != EOF && c != '\n');
        }
        else
            str[i++] = c;

    DDSIP_param->cpxno = 0;
    // If general CPLEX-parameters are specified
    if (find)
    {
        DDSIP_param->cpxwhich = (int *) DDSIP_Alloc (sizeof (int), DDSIP_maxparam, "cpxwhich(ReadCpxPara)");
        DDSIP_param->cpxwhat = (double *) DDSIP_Alloc (sizeof (double), DDSIP_maxparam, "cpxwhat(ReadCpxPara)");
        cnt = 0;
        if (fscanf (specfile, "%s", str) != 1)
        {
            printf ("*Error: could not read string in CPLEX parameter section\n");
        }
        // Read as long as the following patterns aren't met
        while (strncmp (str, "CPLEXLB", 7)   && strncmp (str, "CPLEXUB", 7) &&
                strncmp (str, "CPLEX2LB", 9)  && strncmp (str, "CPLEX2UB", 9) &&
                strncmp (str, "CPLEXEEV", 8)  && strncmp (str, "CPLEXDUAL", 9) &&
                strncmp (str, "CPLEXEND", 8)  && cnt < DDSIP_maxparam && atoi (str))
        {
            DDSIP_param->cpxwhich[cnt] = atoi (str);
            if (fscanf (specfile, "%lf", &DDSIP_param->cpxwhat[cnt++]) != 1)
            {
                printf ("*Error: could not read value in CPLEX parameter section for parameter %d\n", DDSIP_param->cpxwhich[cnt-1]);
            }
            if (cnt >= DDSIP_maxparam)
            {
                printf ("\nFirst %d parameters in section CPLEXBEGIN read, remaining neglected.\n", DDSIP_maxparam);
                break;
            }

            if (DDSIP_param->cpxwhich[cnt - 1] == CPX_PARAM_MIPORDIND)
                DDSIP_param->cpxorder = DDSIP_param->cpxwhat[cnt - 1];
            // Read till end of line
            while ((c = fgetc (specfile)) != EOF && c != '\n');
            if (fscanf (specfile, "%s", str) != 1)
            {
                printf ("*Error: could not read string in CPLEX parameter section\n");
            }
        }
        DDSIP_param->cpxno = cnt;
        if (cnt)
        {
            DDSIP_param->cpxisdbl = (int *) DDSIP_Alloc (sizeof (int), DDSIP_param->cpxno, "cpxisdbl(ReadCpxPara)");
            for(k=0; k<cnt; k++)
                if ((status=CPXgetparamtype(DDSIP_env, DDSIP_param->cpxwhich[k], &j)))
                {
                    printf ("XXX Warning CPLEX returned an error %d in the inquiry of type of parameter %d.\n",status,DDSIP_param->cpxwhich[k]);
                }
                else
                {
                    DDSIP_param->cpxisdbl[k] = j-1;
                }
        }
    }

    find = 0;
    sprintf (pattern, "CPLEXLB");
    // Search for 'CPLEXLB' from beginning of specfile
    rewind (specfile);
    while (!find && (c = fgetc (specfile)) != EOF)
        if (c == '\n' || c == ' ' || c == '*' )
        {
            str[i] = '\0';
            if (!strncmp (str, pattern, strlen (pattern)))
                find = 1;
            i = 0;
            // Read till end of line
            if (c != '\n')
                while ((c = fgetc (specfile)) != EOF && c != '\n');
        }
        else
            str[i++] = c;
    DDSIP_param->cpxnolb = 0;
    // If 'LowerBound'-CPLEX-parameters are specified
    if (find)
    {
        DDSIP_param->cpxlbwhich = (int *) DDSIP_Alloc (sizeof (int), DDSIP_maxparam, "cpxlbwhich(ReadCpxPara)");
        DDSIP_param->cpxlbwhat = (double *) DDSIP_Alloc (sizeof (double), DDSIP_maxparam, "cpxlbwhat(ReadCpxPara)");
        cnt = 0;
        if (fscanf (specfile, "%s", str) != 1)
        {
            printf ("*Error: could not read string in CPLEX parameter section\n");
        }
        // Read as long as the other patterns aren't met
        while (strncmp (str, "CPLEXUB", 7)  &&
                strncmp (str, "CPLEX2LB", 8) && strncmp (str, "CPLEX2UB", 8)  &&
                strncmp (str, "CPLEXEEV", 8) && strncmp (str, "CPLEXDUAL", 9) &&
                strncmp (str, "CPLEXEND", 8) && cnt < DDSIP_maxparam && atoi (str))
        {
            if (str[0] != '*')
            {
                DDSIP_param->cpxlbwhich[cnt] = atoi (str);
                if (fscanf (specfile, "%lf", &DDSIP_param->cpxlbwhat[cnt++]) != 1)
                {
                    printf ("*Error: could not read value in CPLEXLB parameter section for parameter %d\n", DDSIP_param->cpxlbwhich[cnt-1]);
                }
                if (cnt >= DDSIP_maxparam)
                {
                    printf ("\nFirst %d parameters in section CPLEXLB read, remaining neglected.\n", DDSIP_maxparam);
                    break;
                }
            }
            // Read till end of line
            while ((c = fgetc (specfile)) != EOF && c != '\n');
            if (fscanf (specfile, "%s", str) != 1)
            {
                printf ("*Error: could not read string in CPLEX parameter section\n");
            }
        }
        DDSIP_param->cpxnolb = cnt;
        if (cnt)
        {
            DDSIP_param->cpxlbisdbl = (int *) DDSIP_Alloc (sizeof (int), DDSIP_param->cpxnolb, "cpxlbisdbl(ReadCpxPara)");
            for(k=0; k<cnt; k++)
                if ((status=CPXgetparamtype(DDSIP_env, DDSIP_param->cpxlbwhich[k], &j)))
                {
                    printf ("XXX Warning CPLEX returned an error %d in the inquiry of type of parameter %d.\n",status,DDSIP_param->cpxlbwhich[k]);
                }
                else
                {
                    DDSIP_param->cpxlbisdbl[k] = j-1;
                }
        }
    }

    find = 0;
    sprintf (pattern, "CPLEX2LB");
    // Search for 'CPLEX2LB' from beginning of specfile
    rewind (specfile);
    while (!find && (c = fgetc (specfile)) != EOF)
        if (c == '\n' || c == ' ' || c == '*' )
        {
            str[i] = '\0';
            if (!strncmp (str, pattern, strlen (pattern)))
                find = 1;
            i = 0;
            // Read till end of line
            if (c != '\n')
                while ((c = fgetc (specfile)) != EOF && c != '\n');
        }
        else
            str[i++] = c;

    DDSIP_param->cpxnolb2 = 0;
    // If 'LowerBound 2'-CPLEX-parameters are specified
    if (find)
    {
        DDSIP_param->cpxlbwhich2 = (int *) DDSIP_Alloc (sizeof (int), DDSIP_maxparam, "cpxlbwhich(ReadCpxPara)");
        DDSIP_param->cpxlbwhat2 = (double *) DDSIP_Alloc (sizeof (double), DDSIP_maxparam, "cpxlbwhat(ReadCpxPara)");
        cnt = 0;
        if (fscanf (specfile, "%s", str) != 1)
        {
            printf ("*Error: could not read string in CPLEX parameter section\n");
        }
        // Read as long as the other patterns aren't met
        while (strncmp (str, "CPLEXLB", 7)  && strncmp (str, "CPLEXUB", 7)   &&
                strncmp (str, "CPLEX2UB", 8) &&
                strncmp (str, "CPLEXEEV", 8) && strncmp (str, "CPLEXDUAL", 9) &&
                strncmp (str, "CPLEXEND", 8) && cnt < DDSIP_maxparam && atoi (str))
        {
            if (str[0] != '*')
            {
                DDSIP_param->cpxlbwhich2[cnt] = atoi (str);
                if (fscanf (specfile, "%lf", &DDSIP_param->cpxlbwhat2[cnt++]) != 1)
                {
                    printf ("*Error: could not read value in CPLEXLB2 parameter section for parameter %d\n", DDSIP_param->cpxlbwhich2[cnt-1]);
                }
                if (cnt >= DDSIP_maxparam)
                {
                    printf ("\nFirst %d parameters in section CPLEX2LB read, remaining neglected.\n", DDSIP_maxparam);
                    break;
                }
            }
            // Read till end of line
            while ((c = fgetc (specfile)) != EOF && c != '\n');
            if (fscanf (specfile, "%s", str) != 1)
            {
                printf ("*Error: could not read string in CPLEX parameter section\n");
            }
        }
        DDSIP_param->cpxnolb2 = cnt;
        if (cnt)
        {
            DDSIP_param->cpxlbisdbl2 = (int *) DDSIP_Alloc (sizeof (int), DDSIP_param->cpxnolb2, "cpxlbisdbl2(ReadCpxPara)");
            for(k=0; k<cnt; k++)
                if ((status=CPXgetparamtype(DDSIP_env, DDSIP_param->cpxlbwhich2[k], &j)))
                {
                    printf ("XXX Warning CPLEX returned an error %d in the inquiry of type of parameter %d.\n",status,DDSIP_param->cpxlbwhich2[k]);
                }
                else
                {
                    DDSIP_param->cpxlbisdbl2[k] = j-1;
                }
        }
    }

    find = 0;
    sprintf (pattern, "CPLEXUB");
    // Search for 'CPLEXUB' from beginning of specfile
    rewind (specfile);
    while (!find && (c = fgetc (specfile)) != EOF)
        if (c == '\n' || c == ' ' || c == '*' )
        {
            str[i] = '\0';
            if (!strncmp (str, pattern, strlen (pattern)))
                find = 1;
            i = 0;
            // Read till end of line
            if (c != '\n')
                while ((c = fgetc (specfile)) != EOF && c != '\n');
        }
        else
            str[i++] = c;
    DDSIP_param->cpxnoub = 0;
    // If 'UpperBound'-CPLEX-parameters are specified
    if (find)
    {
        DDSIP_param->cpxubwhich = (int *) DDSIP_Alloc (sizeof (int), DDSIP_maxparam, "cpxlbwhich(ReadCpxPara)");
        DDSIP_param->cpxubwhat = (double *) DDSIP_Alloc (sizeof (double), DDSIP_maxparam, "cpxlbwhat(ReadCpxPara)");
        cnt = 0;
        if (fscanf (specfile, "%s", str) != 1)
        {
            printf ("*Error: could not read string in CPLEX parameter section\n");
        }
        // Read as long as the other patterns aren't met
        while (strncmp (str, "CPLEXLB", 7)  &&
                strncmp (str, "CPLEX2LB", 8) && strncmp (str, "CPLEX2UB", 8) &&
                strncmp (str, "CPLEXEEV", 8) && strncmp (str, "CPLEXDUAL", 9) &&
                strncmp (str, "CPLEXEND", 8) && cnt < DDSIP_maxparam && atoi (str))
        {
            if (str[0] != '*')
            {
                DDSIP_param->cpxubwhich[cnt] = atoi (str);
                if(fscanf (specfile, "%lf", &DDSIP_param->cpxubwhat[cnt++]) != 1)
                {
                    printf ("*Error: could not read value in CPLEXUB parameter section for parameter %d\n", DDSIP_param->cpxubwhich[cnt-1]);
                }
                if (cnt >= DDSIP_maxparam)
                {
                    printf ("\nFirst %d parameters in section CPLEXUB read, remaining neglected.\n", DDSIP_maxparam);
                    break;
                }
            }
            // Read till end of line
            while ((c = fgetc (specfile)) != EOF && c != '\n');
            if (fscanf (specfile, "%s", str) != 1)
            {
                printf ("*Error: could not read string in CPLEX parameter section\n");
            }
        }
        DDSIP_param->cpxnoub = cnt;
        if (cnt)
        {
            DDSIP_param->cpxubisdbl = (int *) DDSIP_Alloc (sizeof (int), DDSIP_param->cpxnoub, "cpxubisdbl(ReadCpxPara)");
            for(k=0; k<cnt; k++)
                if ((status=CPXgetparamtype(DDSIP_env, DDSIP_param->cpxubwhich[k], &j)))
                {
                    printf ("XXX Warning CPLEX returned an error %d in the inquiry of type of parameter %d.\n",status,DDSIP_param->cpxubwhich[k]);
                }
                else
                {
                    DDSIP_param->cpxubisdbl[k] = j-1;
                }
        }
    }

    find = 0;
    sprintf (pattern, "CPLEX2UB");
    // Search for 'CPLEX2UB' from beginning of specfile
    rewind (specfile);
    while (!find && (c = fgetc (specfile)) != EOF)
        if (c == '\n' || c == ' ' || c == '*' )
        {
            str[i] = '\0';
            if (!strncmp (str, pattern, strlen (pattern)))
                find = 1;
            i = 0;
            // Read till end of line
            if (c != '\n')
                while ((c = fgetc (specfile)) != EOF && c != '\n');
        }
        else
            str[i++] = c;

    DDSIP_param->cpxnoub2 = 0;
    // If 'UpperBound 2'-CPLEX-parameters are specified
    if (find)
    {
        DDSIP_param->cpxubwhich2 = (int *) DDSIP_Alloc (sizeof (int), DDSIP_maxparam, "cpxubwhich2(ReadCpxPara)");
        DDSIP_param->cpxubwhat2 = (double *) DDSIP_Alloc (sizeof (double), DDSIP_maxparam, "cpxubwhat2(ReadCpxPara)");
        cnt = 0;
        if (fscanf (specfile, "%s", str) != 1)
        {
            printf ("*Error: could not read string in CPLEX parameter section\n");
        }
        // Read as long as the other patterns aren't met
        while (strncmp (str, "CPLEXLB", 9)  && strncmp (str, "CPLEXUB", 7) &&
                strncmp (str, "CPLEX2LB", 8) &&
                strncmp (str, "CPLEXEEV", 8) && strncmp (str, "CPLEXDUAL", 9) &&
                strncmp (str, "CPLEXEND", 8) && cnt < DDSIP_maxparam && atoi (str))
        {
            if (str[0] != '*')
            {
                DDSIP_param->cpxubwhich2[cnt] = atoi (str);
                if (fscanf (specfile, "%lf", &DDSIP_param->cpxubwhat2[cnt++]) != 1)
                {
                    printf ("*Error: could not read value in CPLEX2UB parameter section for parameter %d\n", DDSIP_param->cpxubwhich2[cnt-1]);
                }
                if (cnt >= DDSIP_maxparam)
                {
                    printf ("\nFirst %d parameters in section CPLEX2UB read, remaining neglected.\n", DDSIP_maxparam);
                    break;
                }
            }
            // Read till end of line
            while ((c = fgetc (specfile)) != EOF && c != '\n');
            if (fscanf (specfile, "%s", str) != 1)
            {
                printf ("*Error: could not read string in CPLEX parameter section\n");
            }
        }
        DDSIP_param->cpxnoub2 = cnt;
        if (cnt)
        {
            DDSIP_param->cpxubisdbl2 = (int *) DDSIP_Alloc (sizeof (int), DDSIP_param->cpxnoub2, "cpxubisdbl2(ReadCpxPara)");
            for(k=0; k<cnt; k++)
                if ((status=CPXgetparamtype(DDSIP_env, DDSIP_param->cpxubwhich2[k], &j)))
                {
                    printf ("XXX Warning CPLEX returned an error %d in the inquiry of type of parameter %d.\n",status,DDSIP_param->cpxubwhich2[k]);
                }
                else
                {
                    DDSIP_param->cpxubisdbl2[k] = j-1;
                }
        }
    }

    find = 0;
    sprintf (pattern, "CPLEXEEV");
    // Search for 'CPLEXEEV' from beginning of specfile
    rewind (specfile);
    while (!find && (c = fgetc (specfile)) != EOF)
        if (c == '\n' || c == ' ' || c == '*' )
        {
            str[i] = '\0';
            if (!strncmp (str, pattern, strlen (pattern)))
                find = 1;
            i = 0;
            // Read till end of line
            if (c != '\n')
                while ((c = fgetc (specfile)) != EOF && c != '\n');
        }
        else
            str[i++] = c;

    DDSIP_param->cpxnoeev = 0;
    // If 'EEV'-CPLEX-parameters are specified
    if (find)
    {
        DDSIP_param->cpxeevwhich = (int *) DDSIP_Alloc (sizeof (int), DDSIP_maxparam, "cpxeevwhich(ReadCpxPara)");
        DDSIP_param->cpxeevwhat = (double *) DDSIP_Alloc (sizeof (double), DDSIP_maxparam, "cpxeevwhat(ReadCpxPara)");
        cnt = 0;
        if (fscanf (specfile, "%s", str) != 1)
        {
            printf ("*Error: could not read string in CPLEX parameter section\n");
        }
        // Read as long as the other patterns aren't met
        while (strncmp (str, "CPLEXLB", 7)  && strncmp (str, "CPLEXUB", 7)   &&
                strncmp (str, "CPLEX2LB", 8) && strncmp (str, "CPLEX2UB", 8) &&
                strncmp (str, "CPLEXDUAL", 9) &&
                strncmp (str, "CPLEXEND", 8) && cnt < DDSIP_maxparam && atoi (str))
        {
            if (str[0] != '*')
            {
                DDSIP_param->cpxeevwhich[cnt] = atoi (str);
                if (fscanf (specfile, "%lf", &DDSIP_param->cpxeevwhat[cnt++]) != 1)
                {
                    printf ("*Error: could not read value in CPLEXEEV parameter section for parameter %d\n", DDSIP_param->cpxeevwhich[cnt-1]);
                }
                if (cnt >= DDSIP_maxparam)
                {
                    printf ("\nFirst %d parameters in section CPLEXEEV read, remaining neglected.\n", DDSIP_maxparam);
                    break;
                }
            }
            // Read till end of line
            while ((c = fgetc (specfile)) != EOF && c != '\n');
            if (fscanf (specfile, "%s", str) != 1)
            {
                printf ("*Error: could not read string in CPLEX parameter section\n");
            }
        }
        DDSIP_param->cpxnoeev = cnt;
        if (cnt)
        {
            DDSIP_param->cpxeevisdbl = (int *) DDSIP_Alloc (sizeof (int), DDSIP_param->cpxnoeev, "cpxeevisdbl(ReadCpxPara)");
            for(k=0; k<cnt; k++)
                if ((status=CPXgetparamtype(DDSIP_env, DDSIP_param->cpxeevwhich[k], &j)))
                {
                    printf ("XXX Warning CPLEX returned an error %d in the inquiry of type of parameter %d.\n",status,DDSIP_param->cpxeevwhich[k]);
                }
                else
                {
                    DDSIP_param->cpxeevisdbl[k] = j-1;
                }
        }
    }

    find = 0;
    sprintf (pattern, "CPLEXDUAL");
    // Search for 'CPLEXDUAL' from beginning of specfile
    rewind (specfile);
    while (!find && (c = fgetc (specfile)) != EOF)
        if (c == '\n' || c == ' ' || c == '*' )
        {
            str[i] = '\0';
            if (!strncmp (str, pattern, strlen (pattern)))
                find = 1;
            i = 0;
            // Read till end of line
            if (c != '\n')
                while ((c = fgetc (specfile)) != EOF && c != '\n');
        }
        else
            str[i++] = c;

    DDSIP_param->cpxnodual = 0;
    // If 'DUAL'-CPLEX-parameters are specified
    if (find)
    {
        DDSIP_param->cpxdualwhich = (int *) DDSIP_Alloc (sizeof (int), DDSIP_maxparam, "cpxeevwhich(ReadCpxPara)");
        DDSIP_param->cpxdualwhat = (double *) DDSIP_Alloc (sizeof (double), DDSIP_maxparam, "cpxeevwhat(ReadCpxPara)");
        cnt = 0;
        if (fscanf (specfile, "%s", str) != 1)
        {
            printf ("*Error: could not read string in CPLEX parameter section\n");
        }
        // Read as long as the other patterns aren't met
        while (strncmp (str, "CPLEXLB", 7)  && strncmp (str, "CPLEXUB", 7)   &&
                strncmp (str, "CPLEX2LB", 8) && strncmp (str, "CPLEX2UB", 8) &&
                strncmp (str, "CPLEXEEV", 8) &&
                strncmp (str, "CPLEXEND", 8) && cnt < DDSIP_maxparam && atoi (str))
        {
            if (str[0] != '*')
            {
                DDSIP_param->cpxdualwhich[cnt] = atoi (str);
                if (fscanf (specfile, "%lf", &DDSIP_param->cpxdualwhat[cnt++]) != 1)
                {
                    printf ("*Error: could not read value in CPLEXDUAL parameter section for parameter %d\n", DDSIP_param->cpxdualwhich[cnt-1]);
                }
                if (cnt >= DDSIP_maxparam)
                {
                    printf ("\nFirst %d parameters in section CPLEXDUAL read, remaining neglected.\n", DDSIP_maxparam);
                    break;
                }
            }
            // Read till end of line
            while ((c = fgetc (specfile)) != EOF && c != '\n');
            if (fscanf (specfile, "%s", str) != 1)
            {
                printf ("*Error: could not read string in CPLEX parameter section\n");
            }
        }
        DDSIP_param->cpxnodual = cnt;
        if (cnt)
        {
            DDSIP_param->cpxdualisdbl = (int *) DDSIP_Alloc (sizeof (int), DDSIP_param->cpxnodual, "cpxdualisdbl(ReadCpxPara)");
            for(k=0; k<cnt; k++)
                if ((status=CPXgetparamtype(DDSIP_env, DDSIP_param->cpxdualwhich[k], &j)))
                {
                    printf ("XXX Warning CPLEX returned an error %d in the inquiry of type of parameter %d.\n",status,DDSIP_param->cpxdualwhich[k]);
                    DDSIP_param->cpxdualisdbl[k] = -1;
                }
                else
                {
                    DDSIP_param->cpxdualisdbl[k] = j-1;
                }
        }
    }

    find = 0;
    sprintf (pattern, "CPLEX2DUAL");
    // Search for 'CPLEX2DUAL' from beginning of specfile
    rewind (specfile);
    while (!find && (c = fgetc (specfile)) != EOF)
        if (c == '\n' || c == ' ' || c == '*' )
        {
            str[i] = '\0';
            if (!strncmp (str, pattern, strlen (pattern)))
                find = 1;
            i = 0;
            // Read till end of line
            if (c != '\n')
                while ((c = fgetc (specfile)) != EOF && c != '\n');
        }
        else
            str[i++] = c;

    DDSIP_param->cpxnodual2 = 0;
    // If 'DUAL'-CPLEX-parameters are specified
    if (find)
    {
        DDSIP_param->cpxdualwhich2 = (int *) DDSIP_Alloc (sizeof (int), DDSIP_maxparam, "cpxeevwhich(ReadCpxPara)");
        DDSIP_param->cpxdualwhat2 = (double *) DDSIP_Alloc (sizeof (double), DDSIP_maxparam, "cpxeevwhat(ReadCpxPara)");
        cnt = 0;
        if (fscanf (specfile, "%s", str) != 1)
        {
            printf ("*Error: could not read string in CPLEX parameter section\n");
        }
        // Read as long as the other patterns aren't met
        while (strncmp (str, "CPLEXLB", 7)  && strncmp (str, "CPLEXUB", 7)   &&
                strncmp (str, "CPLEX2LB", 8) && strncmp (str, "CPLEX2UB", 8) &&
                strncmp (str, "CPLEXEEV", 8) &&
                strncmp (str, "CPLEXEND", 8) && cnt < DDSIP_maxparam && atoi (str))
        {
            if (str[0] != '*')
            {
                DDSIP_param->cpxdualwhich2[cnt] = atoi (str);
                if (fscanf (specfile, "%lf", &DDSIP_param->cpxdualwhat2[cnt++]) != 1)
                {
                    printf ("*Error: could not read value in CPLEXDUAL parameter section for parameter %d\n", DDSIP_param->cpxdualwhich[cnt-1]);
                }
                if (cnt >= DDSIP_maxparam)
                {
                    printf ("\nFirst %d parameters in section CPLEXDUAL read, remaining neglected.\n", DDSIP_maxparam);
                    break;
                }
            }
            // Read till end of line
            while ((c = fgetc (specfile)) != EOF && c != '\n');
            if (fscanf (specfile, "%s", str) != 1)
            {
                printf ("*Error: could not read string in CPLEX parameter section\n");
            }
        }
        DDSIP_param->cpxnodual2 = cnt;
        if (cnt)
        {
            DDSIP_param->cpxdualisdbl2 = (int *) DDSIP_Alloc (sizeof (int), DDSIP_param->cpxnodual2, "cpxdualisdbl(ReadCpxPara)");
            for(k=0; k<cnt; k++)
                if ((status=CPXgetparamtype(DDSIP_env, DDSIP_param->cpxdualwhich2[k], &j)))
                {
                    printf ("XXX Warning CPLEX returned an error %d in the inquiry of type of parameter %d.\n",status,DDSIP_param->cpxdualwhich2[k]);
                    DDSIP_param->cpxdualisdbl2[k] = -1;
                }
                else
                {
                    DDSIP_param->cpxdualisdbl2[k] = j-1;
                }
        }
    }

    return 0;
}

//==========================================================================
// Function reads specifications from specfile into structure 'param'
int
DDSIP_ReadSpec ()
{
    char fname[DDSIP_ln_fname];
    FILE *specfile;
    char *ref_point_file;
    FILE *reffile;
    int i;
    double tmp;

    printf ("Enter specification file name:  ");
    i = scanf ("%s", fname);

    if (i == 0 || (specfile = fopen (fname, "r")) == NULL)
    {
        printf ("\nERROR: Cannot open `%s' (ReadSpec) \n", fname);
        return -1;
    }
    printf ("\n\t Reading specifications from `%s'.\n", fname);

    // Read parameters
    fprintf (DDSIP_outfile, "PARAMETERS READ FROM `%s': \n", fname);

    // Two-stage stochastic programming parameters
    fprintf (DDSIP_outfile, "-----------------------------------------------------------\n");
    fprintf (DDSIP_outfile, "-TWO-STAGE STOCHASTIC PROGRAM\n\n");

    DDSIP_param->firstcon = floor (DDSIP_ReadDbl (specfile, "FIRSTC", " FIRST STAGE CONSTRAINTS", 0., 1, 0., DDSIP_bigint) + 0.1);
    DDSIP_param->firstvar = floor (DDSIP_ReadDbl (specfile, "FIRSTV", " FIRST STAGE VARIABLES", 1., 1, 1., DDSIP_bigint) + 0.1);
    DDSIP_param->seccon = floor (DDSIP_ReadDbl (specfile, "SECCON", " SECOND STAGE CONSTRAINTS", 0., 1, 0., DDSIP_bigint) + 0.1);
    DDSIP_param->secvar = floor (DDSIP_ReadDbl (specfile, "SECVAR", " SECOND STAGE VARIABLES", 1., 1, 0., DDSIP_bigint) + 0.1);
    DDSIP_param->prefix = DDSIP_ReadString (specfile, "PREFIX", " PREFIX FOR FIRST STAGE VARIABLE NAMES");
    DDSIP_param->postfix = DDSIP_ReadString (specfile, "POSTFIX", " POSTFIX FOR FIRST STAGE VARIABLE NAMES");
#ifdef EXPLICITPOSTFIX
    if (!DDSIP_param->postfix && !DDSIP_param->prefix)
    {
        printf (" *** ERROR: Neither prefix nor postfix for the first stage variables are specified.\n");
        fprintf (DDSIP_outfile," *** ERROR: Neither prefix nor postfix for the first stage variables are specified.\n");
        exit (1);
    }
    else if (DDSIP_param->postfix && DDSIP_param->prefix)
    {
        printf (" *** ERROR: Either prefix or postfix for the first stage variables could be specified, not both  - Exiting.\n");
        fprintf (DDSIP_outfile," *** ERROR: Either prefix or postfix for the first stage variables could be specified, not both  - Exiting.\n");
        exit (1);
    }
#else
    // Standard behaviour: postfix is '01'
    if (!DDSIP_param->prefix && !DDSIP_param->postfix)
    {
        printf (" *** WARNING: Neither prefix nor postfix for the first stage variables are specified.\n ***          Assuimg the postfix '01'.\n");
        fprintf (DDSIP_outfile," *** WARNING: Neither prefix nor postfix for the first stage variables are specified.\n ***          Assuimg the postfix '01'.\n");
        DDSIP_param->postfix = DDSIP_Alloc (sizeof(char),3,"ReadSpecs(DDSIP_param->postfix)");
        sprintf (DDSIP_param->postfix,"01");
    }
    else if (DDSIP_param->postfix && DDSIP_param->prefix)
    {
        printf (" *** ERROR: Either prefix or postfix for the first stage variables could be specified, not both  - Exiting.\n");
        fprintf (DDSIP_outfile," *** ERROR: Either prefix or postfix for the first stage variables could be specified, not both  - Exiting.\n");
        exit (1);
    }
#endif

#ifndef NEOS
    DDSIP_param->scenarios = floor (DDSIP_ReadDbl (specfile, "SCENAR", " SCENARIOS", 1., 1, 1., DDSIP_bigint) + 0.1);
#else
    // for NEOS server: impossible to use DDSIP for just one scenario
    DDSIP_param->scenarios = floor ( DDSIP_ReadDbl(specfile,"SCENAR"," SCENARIOS",2.,1,2.,DDSIP_bigint) + 0.1);
#endif

    DDSIP_param->stocrhs = floor (DDSIP_ReadDbl (specfile, "STOCRH", " STOCHASTIC RHS", 0., 1, 0., DDSIP_bigint) + 0.1);
    DDSIP_param->stoccost = floor (DDSIP_ReadDbl (specfile, "STOCCO", " STOCHASTIC COST COEFFICIENTS", 0., 1, 0., DDSIP_bigint) + 0.1);
    DDSIP_param->stocmat = floor (DDSIP_ReadDbl (specfile, "STOCMA", " STOCHASTIC MATRIX ENTRIES", 0., 1, 0., DDSIP_bigint) + 0.1);


    // Cplex parameters, output somewhere else
    DDSIP_ReadCpxPara (specfile);

    // not implemented: SMPS reading
    //  DDSIP_param->smps = floor ( DDSIP_ReadDbl(specfile,"SMPSFO"," READ SMPS FORMAT",0.,1,0.,1.) + 0.1);

    //b&b parameters
    fprintf (DDSIP_outfile, "-----------------------------------------------------------\n");
    fprintf (DDSIP_outfile, "-DUAL DECOMPOSITION PROCEDURE\n\n");

    DDSIP_param->outlev    = floor (DDSIP_ReadDbl (specfile, "OUTLEV", " OUTPUT LEVEL", 0., 1, 0.,100.) + 0.1);
    DDSIP_param->files     = floor (DDSIP_ReadDbl (specfile, "OUTFIL", " OUTPUT FILES LEVEL", 1., 1, 0., 6.) + 0.1);
    DDSIP_param->logfreq   = floor (DDSIP_ReadDbl (specfile, "LOGFRE", " LOG FREQUENCY", 1., 1, 0., DDSIP_bigint) + 0.1);
    DDSIP_param->nodelim   = floor (DDSIP_ReadDbl (specfile, "NODELI", " NODE LIMIT", 10000., 1, 1., DDSIP_bigint) + 0.1);
    // uncomment for default timelimit: 24h
    //DDSIP_param->timelim   = DDSIP_ReadDbl (specfile, "TIMELI", " TIME LIMIT", 86400., 0, 0., DDSIP_infty);
    // default timelimit: 14 days
    DDSIP_param->timelim   = DDSIP_ReadDbl (specfile, "TIMELI", " TIME LIMIT", 1209600., 0, 0., DDSIP_infty);
    DDSIP_param->absgap    = DDSIP_ReadDbl (specfile, "ABSOLU", " ABSOLUTE GAP", 0., 0, 0., DDSIP_infty);
    DDSIP_param->relgap    = DDSIP_ReadDbl (specfile, "RELATI", " RELATIVE GAP", 0.0001, 0, 0., 1.);
    DDSIP_param->expected  = floor (DDSIP_ReadDbl (specfile, "EEVPRO", " EXPECTED VALUE PROBLEM", 0., 1, 0., 1.) + 0.1);
    // Write deterministic DDSIP_equivalent (only expectation-based case so far)
    //DDSIP_param->write_detequ = floor (DDSIP_ReadDbl (specfile, "DETEQU", " WRITE DETERMINISTIC EQUIVALENT", 0., 1, 0., 1.) + 0.1);
    //Disabled due to the canges in the input format and lifting the requirement of ordering the constaints according to stages
    DDSIP_param->write_detequ = 0;

    DDSIP_param->order     = floor (DDSIP_ReadDbl (specfile, "PORDER", " PRIORITY ORDER", 0., 1, 0., 1.) + 0.1);
    DDSIP_param->advstart  = floor (DDSIP_ReadDbl (specfile, "STARTI", " INITIAL SOLUTION/BOUND", 0., 1, 0., 1.) + 0.1);
    DDSIP_param->hot       = floor (DDSIP_ReadDbl (specfile, "HOTSTA", " HOT STARTS DURING B&B", 1., 1, 0., 6.) + 0.1);
    if (DDSIP_param->stoccost && DDSIP_param->hot == 2)
    {
        fprintf (DDSIP_outfile, "      HOSTART=2 not implemented for stoch. cost coefficients, resetting to 1.\n");
        DDSIP_param->hot = 1;
    }

    DDSIP_param->branchdir   = floor (DDSIP_ReadDbl (specfile, "BRADIR", " BRANCHING DIRECTION", -1., 1, -1., 1.) + 0.1);
    DDSIP_param->branchstrat = floor (DDSIP_ReadDbl (specfile, "BRASTR", " BRANCHING STRATEGY", 2., 1, 0., 2.) + 0.1);
    DDSIP_param->equalbranch = floor (DDSIP_ReadDbl (specfile, "BRAEQU", " EQUAL DIVIDE BRANCHING", 0., 1, -1., 1.) + 0.1);
    DDSIP_param->intfirst    = floor (DDSIP_ReadDbl (specfile, "INTFIR", " BRANCH INTEGER FIRST", 1., 1, 0., 1.) + 0.1);
    DDSIP_param->boundstrat  = floor (DDSIP_ReadDbl (specfile, "BOUSTR", " BOUNDING STRATEGY", 10., 1, 0., 10.) + 0.1);
    if (DDSIP_param->boundstrat == 10)
        DDSIP_param->bestboundfreq= floor(DDSIP_ReadDbl (specfile, "BESTFR", " BEST BOUND FREQUENCY", 2.*DDSIP_param->scenarios, 1, 0., DDSIP_bigint) + 0.1);
    else
        DDSIP_param->bestboundfreq= 100;
    DDSIP_param->period = floor (DDSIP_ReadDbl (specfile, "PERIOD", " HEUR PERIOD ITERS", 32., 1, 1., 10000.) + 0.1);
    DDSIP_param->rgapsmall = floor (DDSIP_ReadDbl (specfile, "TOLSMA", " HEUR SMALL RGAP ITERS", 16., 1, 1., 1.*DDSIP_param->period) + 0.1);

    // Accuracy, e.g. for the  comparison of double numbers
    DDSIP_param->accuracy    = DDSIP_ReadDbl (specfile, "ACCURA", " ACCURACY", 1.0e-15, 0, 5.e-16, 1.);
    // DDSIP_param->accuracy = DDSIP_Dmax (DDSIP_param->relgap * DDSIP_param->accuracy,1e-8);
    DDSIP_param->brancheps   = DDSIP_ReadDbl (specfile, "EPSILO", " EPSILON", 1.0e-14, 0, 5.e-16, 1.);
    DDSIP_param->nulldisp    = DDSIP_ReadDbl (specfile, "NULLDI", " NULL DISPERSION", 2.e-14, 0, 5.e-16, DDSIP_infty);
    DDSIP_param->watchkappa  = floor (DDSIP_ReadDbl (specfile, "KAPPA", " GATHER KAPPA INFORMATION", 0., 1, 0., 2.) + 0.1);
    DDSIP_param->relax       = floor (DDSIP_ReadDbl (specfile, "RELAXL", " RELAXATION LEVEL", 0., 1, 0., DDSIP_bigint) + 0.1);
    DDSIP_param->noquant     = floor (DDSIP_ReadDbl (specfile, "QUANTI", " NUMBER OF QUANTILES", 10., 1, 0., DDSIP_bigint) + 0.1);
    DDSIP_param->maxinherit = floor (DDSIP_ReadDbl (specfile, "MAXINH", " MAX. LEVEL OF INHERITANCE", 5., 1, 0., 100000.) + 0.1);

    DDSIP_param->heuristic_vector = NULL;
    if ((DDSIP_param->heuristic = floor (DDSIP_ReadDbl (specfile, "HEURIS", " HEURISTIC", 100., 1, 0., 100.) + 0.1)) == 99)
    {
        DDSIP_param->heuristic_vector =
            DDSIP_ReadDblVec (specfile, "HEURIS", " HEURISTICVector", 3., 1, 0., 99., 12, &DDSIP_param->heuristic_num);
        if (DDSIP_param->heuristic_num < 2)
        {
            fprintf (DDSIP_outfile, "      missing list of heuristics to be used, resetting to 3.\n");
            DDSIP_param->heuristic = 3;
        }
    }
    DDSIP_param->heuristic_auto = 0;
    if (DDSIP_param->heuristic == 100)
    {
        DDSIP_param->heuristic_vector = (double *) DDSIP_Alloc (sizeof (double), 12, "values(DDSIP_ReadDblVec)");
        DDSIP_param->heuristic_vector[0] = 100.;
        for (i = 1; i< 4; i++)
        {
            DDSIP_param->heuristic_vector[i]     = i + 3;
            DDSIP_param->heuristic_vector[i + 3] = i;
        }
        for (i = 7; i< 12; i++)
            DDSIP_param->heuristic_vector[i] = i;
        //DDSIP_param->heuristic = 99;
        DDSIP_param->heuristic_num = 11;
        DDSIP_param->heuristic_auto = 1;
    }
    //DDSIP_param->prepro = floor (DDSIP_ReadDbl (specfile, "PREPRO", " PREPROCESSING", 0., 1, 0., 3.) + 0.1);
    DDSIP_param->prepro = 0;
    fprintf (DDSIP_outfile, "\n");


    //risk model
    // A positive parameter riskmod means: LowerBound the mean-risk model DDSIP_min (E + rho * R)
    // A negative parameter riskmod means: LowerBound the risk model  DDSIP_min (R)
    // riskmod=0 means: LowerBound the expected value model  DDSIP_min (E) (default)
    DDSIP_param->riskmod = floor (DDSIP_ReadDbl (specfile, "RISKMO", " RISK MODEL", 0., 1, -DDSIP_maxrisk, DDSIP_maxrisk) + 0.1);

    if (DDSIP_param->riskmod)
    {
        fprintf (DDSIP_outfile, "-----------------------------------------------------------\n");
        fprintf (DDSIP_outfile, "-RISK MODEL\n\n");
        if (DDSIP_param->write_detequ)
        {
            fprintf (DDSIP_outfile,
                     "--- Warning: Building deterministic equivalent not implemented for risk optimization not implemented.\n");
            printf ("--- Warning: Building deterministic equivalent not implemented for risk optimization.\n");
            DDSIP_param->write_detequ = 0;
        }
        if (DDSIP_param->stoccost)
        {
            fprintf (DDSIP_outfile, "XXX Error: Risk optimization not implemented for stochastic cost coefficients.\n");
            printf ("XXX Error: Risk optimization not implemented for stochastic cost coefficients.\n");
            exit (1);
        }
    }
    // Absolute semideviation models is not lsc .....
    if (DDSIP_param->riskmod == -3)
    {
        DDSIP_param->riskmod = 3;
        fprintf (DDSIP_outfile, "*Warning: Illegal parameter setting: MODEL = -3. Reset MODEL to %d\n", DDSIP_param->riskmod);
        printf ("*Warning: Illegal parameter setting: MODEL = -3. Reset MODEL to %d\n", DDSIP_param->riskmod);
    }

    if (DDSIP_param->riskmod)
    {
        // Use algorithm for fsd-consistent risk measures: ExpExc, ExcProb, VaR, TVaR, SemDev
        DDSIP_param->riskalg = DDSIP_ReadDbl (specfile, "RISKAL", " RISK-ALGORITHM", 0., 1, 0., 2.);

        DDSIP_param->scalarization = floor (DDSIP_ReadDbl (specfile, "SCALAR", " SCALARIZATION", 0., 1, 0., 1.) + 0.1);
        if (DDSIP_param->scalarization && (DDSIP_param->riskmod < 0 || DDSIP_param->riskmod > 2))
        {
            fprintf (DDSIP_outfile, "Warning: reference point scalarization implemented only for risk models 1 and 2.\n");
            DDSIP_param->scalarization = 0;
        }
        // SCALAR == 0 - weighted sum,  == 1 - reference point
        if (DDSIP_param->scalarization)
        {
            /*
               if (DDSIP_param->riskalg != 1) {
               fprintf (DDSIP_outfile, "   using FSD-Algorithm for reference point scalarization, resetting RISKALG.\n");
               DDSIP_param->riskalg = 1;
               }
             */
            DDSIP_param->riskweight = 0.0;
            ref_point_file = DDSIP_ReadString (specfile, "REFERE", " REFERENCE POINT FILE");
            if (!(reffile = fopen (ref_point_file, "r")))
            {
                printf ("Could not open reference point file named %s. exit.\n", ref_point_file);
                exit (11);
            }
            if (!(DDSIP_param->ref_point = (double *) calloc (2, sizeof (double))) ||
                    !(DDSIP_param->ref_scale = (double *) calloc (2, sizeof (double))))
            {
                printf ("No memory for reference point, exit.\n");
                exit (1);
            }
            if (fscanf
                    (reffile, " %lf %lf %lf %lf", DDSIP_param->ref_point, DDSIP_param->ref_scale, DDSIP_param->ref_point + 1,
                     DDSIP_param->ref_scale + 1) != 4)
            {
                printf ("Could not read four values from reference point file. exit.\n");
                exit (11);
            }
            fclose (reffile);
            DDSIP_Free ((void **) &ref_point_file);
            DDSIP_param->ref_eps = DDSIP_ReadDbl (specfile, "REFEPS", " REFERENCE FUNCTION EPSILON", 1e-4, 0, 1.e-9, 1.);
            printf ("   ref. expected value %12.6g,  scale %8.4g\n", DDSIP_param->ref_point[0], DDSIP_param->ref_scale[0]);
            printf ("   ref. risk measure   %12.6g,  scale %8.4g\n", DDSIP_param->ref_point[1], DDSIP_param->ref_scale[1]);
            fprintf (DDSIP_outfile, "   ref. expected value %12.6g,  scale %8.4g\n", DDSIP_param->ref_point[0], DDSIP_param->ref_scale[0]);
            fprintf (DDSIP_outfile, "   ref. risk measure   %12.6g,  scale %8.4g\n", DDSIP_param->ref_point[1], DDSIP_param->ref_scale[1]);
        }
        else
            DDSIP_param->riskweight = DDSIP_ReadDbl (specfile, "WEIGHT", " RISK WEIGHT", 0.9, 0, 0., DDSIP_infty);

        // Target - needed for models 1, 2, and 3 and used for evaluation of all risk measures
        DDSIP_param->risktarget = DDSIP_ReadDbl (specfile, "TARGET", " RISK TARGET", -DDSIP_infty, 0, -DDSIP_infty, DDSIP_infty);
        // Absolute semideviation models is not lsc for weight>1 .....
        if (DDSIP_param->riskmod == 3 && DDSIP_param->riskweight > 1)
        {
            DDSIP_param->riskweight = 1;
            printf ("*Warning: Illegal parameter combination: MODEL = 3,  WEIGHT > 1. Reset WEIGTH to %f.\n",
                    DDSIP_param->riskweight);
        }
        // Weight is irrelevant for pure risk models, but used here and there
        else if (DDSIP_param->riskmod < 0)
            DDSIP_param->riskweight = 1.0;

        // Excess probabilities, Worst case costs, Tail value-at-risk
        //if (abs (DDSIP_param->riskmod) == 2 || abs (DDSIP_param->riskmod) == 4 || abs (DDSIP_param->riskmod) == 5)
        if (DDSIP_param->riskmod)
            DDSIP_param->riskM = DDSIP_ReadDbl (specfile, "RISKBM", " RISK BIGM", DDSIP_infty, 0, 0., DDSIP_infty);

        // Worst case costs, Tail value-at-risk: Branch in auxillary variable eta ?
        if (abs (DDSIP_param->riskmod) == 4 || abs (DDSIP_param->riskmod) == 5)
            DDSIP_param->brancheta = DDSIP_ReadDbl (specfile, "BRAETA", " BRANCH ON ETA", 1., 1, 0., 1.);
        // Default DDSIP_value (branch in all variables)
        else
            DDSIP_param->brancheta = 0;

        // Value-at-risk, Tail value-at-risk: Probability level
        DDSIP_param->risklevel = DDSIP_ReadDbl (specfile, "PROBLE", " RISK LEVEL", 0.1, 0, 0., 1.);
        // Excess probabilities, Absolute semideviation: Target
        if (abs (DDSIP_param->riskmod) == 1 || abs (DDSIP_param->riskmod) == 2 || abs (DDSIP_param->riskmod) == 3)
        {
            if (abs (DDSIP_param->riskmod) > 1 && DDSIP_param->riskM < DDSIP_param->risktarget)
            {

                printf ("*Warning: Risk big M parameter %g less than target %g", DDSIP_param->riskM, DDSIP_param->risktarget);
                DDSIP_param->riskM = DDSIP_param->risktarget * 1e2 + 1.e3;
                printf (", resetting RISKBM to %g.\n", DDSIP_param->riskM);
            }
        }

        if (abs (DDSIP_param->riskmod) == 5)
            DDSIP_param->riskvar = 2;
        else if (abs (DDSIP_param->riskmod) == 7 || abs (DDSIP_param->riskmod) == 6 || DDSIP_param->riskmod == 0)
            DDSIP_param->riskvar = 0;
        else
            DDSIP_param->riskvar = 1;

        if ((abs (DDSIP_param->riskmod) == 4 || abs (DDSIP_param->riskmod) == 7) && DDSIP_param->riskalg == 1)
        {
            DDSIP_param->riskalg = 0;
            printf ("*Warning: Illegal parameter setting: RISKALG = 1.\n");
            printf ("          Risk Model %d is not fsd-consistent. Reset RISKALG to %d\n", DDSIP_param->riskmod,
                    DDSIP_param->riskalg);
        }
        if (abs (DDSIP_param->riskmod) == 6 && DDSIP_param->riskalg != 1)
        {
            printf ("*Hint: Risk Model %d is best solved with algorithm for fsd-consistent models. Better use RISKALG 1 instead of %d.\n",
                    DDSIP_param->riskmod, DDSIP_param->riskalg);
        }
        // Ensure consistency
        if (DDSIP_param->riskalg)
        {
            DDSIP_param->riskvar = 0;
            DDSIP_param->brancheta = 0;
        }
    }
    else
        DDSIP_param->riskvar = 0;

    fprintf (DDSIP_outfile, "-----------------------------------------------------------\n");
    DDSIP_param->cb = 0;
    DDSIP_param->prematureStop = 1;
#ifdef CONIC_BUNDLE
    //conic bundle part
    tmp = (DDSIP_param->riskalg == 1 || DDSIP_param->scalarization) ? 0 : -16;
    DDSIP_param->cb = floor (DDSIP_ReadDbl (specfile, "CBFREQ", " CB METHOD IN EVERY ITH NODE", tmp, 1, -DDSIP_bigint, DDSIP_bigint) + 0.1);
    if (DDSIP_param->scalarization && DDSIP_param->cb)
    {
        printf ("*Warning: Conic bundle does not work in conjunction with reference point scalarization!\n");
        printf ("          Reset CBFREQ to 0.\n");
        fprintf (DDSIP_outfile, "*Warning: Conic bundle does not work in conjunction with reference point scalarization!\n");
        fprintf (DDSIP_outfile, "          Reset CBFREQ to 0.\n");
        DDSIP_param->cb = 0;
    }
    if (DDSIP_param->cb)
    {
        if (DDSIP_param->hot == 2 || DDSIP_param->hot > 4)
        {
            fprintf (DDSIP_outfile, "      HOSTART=2 , 5 or 6: lower bounds for scenario problems not valid with changed Lagrangean multipliers, resetting to 4.\n");
            printf ( "      HOSTART=2 , 5 or 6: lower bounds for scenario problems not valid with changed Lagrangean multipliers, resetting to 4.\n");
            DDSIP_param->hot = 4;
        }
        fprintf (DDSIP_outfile, "-----------------------------------------------------------\n");
        fprintf (DDSIP_outfile, "-CONIC BUNDLE\n\n");
    }
    // Algorithm for fsd-consistent risk measures can not be combined with dual method
    //if (DDSIP_param->cb && DDSIP_param->riskalg == 1)
    if (DDSIP_param->cb && DDSIP_param->riskalg)
    {
        fprintf (DDSIP_outfile, "ConicBundle cannot be used in conjunction with RISKALG 1 or 2. Exiting\n");
        printf ("ConicBundle cannot be used in conjunction with RISKALG 1 or 2. Exiting\n");
        fclose (specfile);
        return 133;
    }

    DDSIP_param->prematureStop = 1;
    if (DDSIP_param->cb)
    {
        //DDSIP_param->prematureStop = 0; // would be safer, but after current changes the lower bound used for premature stoppng is chosen with more caution (should work in many cases)
        DDSIP_param->prematureStop = 1;
        DDSIP_param->cbitlim = floor (DDSIP_ReadDbl (specfile, "CBITLI", " CB DESCENT ITERATIONS", 20., 1, 0., DDSIP_bigint) + 0.1);
        if (abs(DDSIP_param->riskmod) == 4 || abs(DDSIP_param->riskmod) == 5)
        {
            printf ("     setting CBRITLIM to 0 due risk model.\n");
            fprintf (DDSIP_outfile, "     setting CBRITLIM to 0 due risk model.\n");
            DDSIP_param->cbrootitlim = 0;
        }
        else
            DDSIP_param->cbrootitlim = floor (DDSIP_ReadDbl (specfile, "CBRITL", " CB DESCENT ITERATIONS IN ROOT", DDSIP_param->cbitlim+10, 1, 0., DDSIP_bigint) + 0.1);

        DDSIP_param->cb_maxsteps  = floor (DDSIP_ReadDbl (specfile, "CBSTEP", " CB MAXSTEPS", 12., 1, 1., 10000.) + 0.1);
        DDSIP_param->cbtotalitlim = floor (DDSIP_ReadDbl (specfile, "CBTOTI", " CB ITERATION LIMIT",5000., 1, 0., DDSIP_bigint) + 0.1);
        DDSIP_param->cbContinuous = floor (DDSIP_ReadDbl (specfile, "CBCONT", " CONTINUOUS CB CALLS", 4., 1, 0., DDSIP_bigint) + 0.1);
        DDSIP_param->cbBreakIters = floor (DDSIP_ReadDbl (specfile, "CBBREA", " BREAK FOR CB CALLS", abs(DDSIP_param->cb) > 28?1.*abs(DDSIP_param->cb):28., 1, 0., DDSIP_bigint) + 0.1);
        if (DDSIP_param->cbBreakIters < abs(DDSIP_param->cb))
        {
            DDSIP_param->cbBreakIters = abs(DDSIP_param->cb);
            printf ("     CBBREAK smaller than CBFREQ, setting CBBREAK = %d.\n", DDSIP_param->cbBreakIters);
            fprintf (DDSIP_outfile, "     CBBREAK smaller than CBFREQ, setting CBBREAK = %d.\n", DDSIP_param->cbBreakIters);
        }
        DDSIP_param->cbrelgap = DDSIP_ReadDbl (specfile, "CBPREC", " CB PRECISION", 1.e-14, 0, 0., DDSIP_infty);
        DDSIP_param->nonant = floor (DDSIP_ReadDbl (specfile, "NONANT", " CB NON-ANTICIPATIVITY", 1., 1, 1., 3.) + 0.1);
        DDSIP_param->cbprint = floor (DDSIP_ReadDbl (specfile, "CBPRIN", " CB PRINT LEVEL", 0., 1, 0., DDSIP_bigint) + 0.1);
        DDSIP_param->cbbundlesz = floor (DDSIP_ReadDbl (specfile, "CBBUNS", " CB MAXIMAL BUNDLE SIZE", 500., 1, 0., DDSIP_bigint) + 0.1);
        //DDSIP_param->cbmaxsubg = floor (DDSIP_ReadDbl (specfile, "CBMAXS", " CB MAXIMAL NO OF SUBGRADIENTS", 1., 1, 0., DDSIP_bigint) + 0.1);
        DDSIP_param->cbmaxsubg = 1;
        DDSIP_param->cbweight = DDSIP_ReadDbl (specfile, "CBWEIG", " CB START WEIGHT", 1., 0, 0., DDSIP_infty);
        DDSIP_param->cbfactor = DDSIP_ReadDbl (specfile, "CBFACT", " FACTOR OF CB START WEIGHT", 0.01, 0, 0., 1.);
        if (abs(DDSIP_param->riskmod) == 4 && DDSIP_param->cbweight < (tmp = DDSIP_param->riskweight*0.5))
        {
            printf ("\n     CBWEIGHT less than 0.5*(risk weight), resetting to %g, CBFACTOR to %g.\n\n", tmp, 1e-1/tmp);
            fprintf (DDSIP_outfile, "\n     CBWEIGHT less than 0.5*(risk weight), resetting to %g, CBFACTOR to %g.\n\n", tmp, 1e-1/tmp);
            DDSIP_param->cbweight = tmp;
            DDSIP_param->cbfactor = 1e-1/tmp;
        }
        DDSIP_param->cb_inherit = floor (DDSIP_ReadDbl (specfile, "CBINHE", " CB INHERIT SOLUTIONS", 0., 1, 0., 1.) + 0.1);
        DDSIP_param->cb_changetol = floor (DDSIP_ReadDbl (specfile, "CBCHAN", " CB CHANGE TOLERANCE", 0., 1, 0., 1.) + 0.1);
        DDSIP_param->cb_reduceWeight = floor (DDSIP_ReadDbl (specfile, "CBREDU", " CB REDUCE WEIGHT", 1., 1, 0., 1.) + 0.1);
        DDSIP_param->cb_increaseWeight = floor (DDSIP_ReadDbl (specfile, "CBINCR", " CB INCREASE WEIGHT", 1., 1, 0., 1.) + 0.1);
    }
#else
//if (DDSIP_param->cb){
//  printf ("\n XXX Found CBFREQ %d with ConicBundle being not avaliable, resetting to 0.\n\n", DDSIP_param->cb);
//  fprintf (DDSIP_outfile, "\n XXX Found CBFREQ %d with ConicBundle being not avaliable, resetting to 0.\n\n", DDSIP_param->cb);
//}
    DDSIP_param->cb = 0;
    DDSIP_param->prematureStop = 1;
#endif
    fprintf (DDSIP_outfile, "-----------------------------------------------------------\n");
    DDSIP_param->prematureStop=floor (DDSIP_ReadDbl (specfile, "PREMAT", " PREMATURE STOP in UpperBound", DDSIP_param->prematureStop, 1, 0., 1.) + 0.1);
    if (DDSIP_param->prematureStop && DDSIP_param->cb)
    {
        printf ("\n XXX CAUTION XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n XXX Usaging premature stop in UpperBound on the basis of lower bounds together with Conic Bundle.\n XXX Maybe with some problems this could falsely diagnose inferiority of heuristic proposals.\n XXX If unsure whether this is the case with your problem, disable by setting 'PREMATURE  0'\n XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
        fprintf (DDSIP_outfile, "\n XXX CAUTION XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n XXX Usaging premature stop in UpperBound on the basis of lower bounds together with Conic Bundle.\n XXX Maybe with some problems this could falsely diagnose inferiority of heuristic proposals.\n XXX If unsure whether this is the case with your problem, disable by setting 'PREMATURE  0'\n XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
    }
    if (DDSIP_param->brancheps > DDSIP_param->nulldisp)
    {
        printf ("*Warning: Branching EPSILON parameter %g greater than NULLDISPERSION %g", DDSIP_param->brancheps, DDSIP_param->nulldisp);
        DDSIP_param->brancheps = 0.95 * DDSIP_param->nulldisp;
        printf (", resetting EPSILON to %g.\n", DDSIP_param->brancheps);
    }

    fclose (specfile);
    return 0;
}

/*==========================================================================*/
/* Function reads model data from model file and opens cplex environment */
int
DDSIP_ReadModel ()
{
    int status = 0, i;
    char *ctmp;
    char *fname = (char *) DDSIP_Alloc (sizeof (char), DDSIP_ln_fname, "fname (ReadModel)");
    // Create cplex problem
    DDSIP_lp = CPXcreateprob (DDSIP_env, &status, "mip");
    if (DDSIP_lp == NULL)
    {
        printf ("ERROR: Failed to create problem.\n");
        return status;
    }

    printf ("Enter model file name:  ");
    i = scanf ("%s", fname);
    if (i == 0)
    {
        printf ("\nError: could not read model file name.\n");
    }

    // Set core file type
    // Types recongnized by CPLEX are possible
    DDSIP_param->coretype = (char *) DDSIP_Alloc (sizeof (char), 9, "DDSIP_param->coretype (ReadModel)");
    ctmp = strrchr (fname, '.');
    strncpy (DDSIP_param->coretype, ctmp, 4);
    if (!strcmp (DDSIP_param->coretype, ".gz"))
    {
        * (ctmp) = '\0';
        strncpy (DDSIP_param->coretype, strrchr (fname, '.'), 4);
        strcat (DDSIP_param->coretype, ".gz");
        printf (" coretype = %s\n", DDSIP_param->coretype);
        * (ctmp) = '.';
    }
    // Invalid file extension ?
    /*    if (strcmp(DDSIP_param->coretype,".MPS") && strcmp(DDSIP_param->coretype,".mps") && */
    /*        strcmp (DDSIP_param->coretype,".SAV") && strcmp(DDSIP_param->coretype,".sav") && */
    /*        strcmp (DDSIP_param->coretype,".RMP") && strcmp(DDSIP_param->coretype,".rmp") && */
    /*        strcmp (DDSIP_param->coretype,".REW") && strcmp(DDSIP_param->coretype,".rew") && */
    /*        strcmp (DDSIP_param->coretype,".RLP") && strcmp(DDSIP_param->coretype,".rlp") && */
    /*        strcmp (DDSIP_param->coretype,".LP")  && strcmp(DDSIP_param->coretype,".lp")) */
    /*      return 135; */

    printf ("\n\t Reading model from `%s'.\n", fname);

    // Read lp-file
    status = CPXreadcopyprob (DDSIP_env, DDSIP_lp, fname, NULL);
    if (status)
    {
        printf ("ERROR: Failed to read problem file %s.\n", fname);
        return status;
    }
    fprintf (DDSIP_outfile, "\nMODEL READ FROM `%s': \n", fname);
    // Read order file if specified
    if (DDSIP_param->cpxorder)
    {
        printf ("Enter file name with priority order for scenario problems:  ");
        i = scanf ("%s", fname);
        if (i == 0)
        {
            printf ("\nError: could not read priority order file name.\n");
        }
        printf ("\n\t Reading order from `%s'.\n", fname);

        status = CPXreadcopyorder (DDSIP_env, DDSIP_lp, fname);
        if (status)
        {
            printf ("ERROR: Failed to read priority order file.\n");
            return status;
        }
    }
    // Write lp-file to check consistency
    if (DDSIP_param->files > 1)
    {
        sprintf (fname, "%s/model%s", DDSIP_outdir, DDSIP_param->coretype);

        status = CPXwriteprob (DDSIP_env, DDSIP_lp, fname, NULL);
        if (status)
        {
            printf ("ERROR: Failed to write problem file '%s', return-code: %d.\n", fname, status);
            return status;
        }
    }

    DDSIP_Free ((void **) &(fname));

    return status;
}

//==========================================================================
// Function reads stochastic data from data files into structure 'data'
int
DDSIP_ReadData ()
{
    int i, j, k, status = 0, maxVarNameLength;

    double probsum;

    char fname[DDSIP_ln_fname];
    char identifier[DDSIP_max_str_ln];
    char checkstr[DDSIP_max_str_ln];
    char tmpdata[DDSIP_max_str_ln];
    char *colstore = NULL, **colname = NULL, *rowstore = NULL, **rowname = NULL;
    

    FILE *datafile;
    FILE *checkfile;

    maxVarNameLength = DDSIP_Imin (128, DDSIP_max_str_ln);

    // get the column and row names of the problem
    colname = (char **) malloc (sizeof(char *) * (DDSIP_param->firstvar + DDSIP_param->secvar));
    colstore = (char *) malloc (sizeof(char) * (DDSIP_param->firstvar + DDSIP_param->secvar)*(maxVarNameLength+1));
    rowname = (char **) malloc (sizeof(char *) * (DDSIP_param->firstcon + DDSIP_param->seccon));
    rowstore = (char *) malloc (sizeof(char) * (DDSIP_param->firstcon + DDSIP_param->seccon)*(maxVarNameLength+1));

    status = CPXgetcolname (DDSIP_env, DDSIP_lp, colname, colstore, (DDSIP_param->firstvar + DDSIP_param->secvar)*(maxVarNameLength+1), &k, 0, (DDSIP_param->firstvar + DDSIP_param->secvar)-1);
    if ( status )
    {
        fprintf (stderr, "XXX ERROR: Failed to get column names.\n");
        fprintf (DDSIP_outfile, "XXX ERROR: Failed to get column names.\n");
        CPXgeterrorstring (DDSIP_env, status, tmpdata);
        fprintf (stderr, "%s", tmpdata);
        exit (1);
    }

    status = CPXgetrowname (DDSIP_env, DDSIP_lp, rowname, rowstore, (DDSIP_param->firstcon + DDSIP_param->seccon)*(maxVarNameLength+1), &k, 0, (DDSIP_param->firstcon + DDSIP_param->seccon)-1);
    if ( status )
    {
        fprintf (stderr, "XXX ERROR: Failed to get row names.\n");
        fprintf (DDSIP_outfile, "XXX ERROR: Failed to get row names.\n");
        CPXgeterrorstring (DDSIP_env, status, tmpdata);
        fprintf (stderr, "%s", tmpdata);
        exit (1);
    }


    // Reading rhs and probabilities
    DDSIP_data->prob = (double *) DDSIP_Alloc (sizeof (double), DDSIP_param->scenarios, "prob(ReadData)");

    if (DDSIP_param->stocrhs)
        DDSIP_data->rhs = (double *) DDSIP_Alloc (sizeof (double), DDSIP_param->scenarios * DDSIP_param->stocrhs, "rhs(ReadData)");

    strcpy (identifier, "sce");

    printf ("Enter data file name (stochastic rhs):  ");
    i = scanf ("%s", fname);

    if (i == 0 || (datafile = fopen (fname, "r")) == NULL)
    {
        printf ("Cannot open '%s' (ReadData)\n", fname);
        return 1;
    }

    printf ("\n\t Reading %d scenarios including\n", DDSIP_param->scenarios);
    if (DDSIP_param->stocrhs)
    {
        printf ("\t\t - %d stochastic rhs from `%s'.\n", DDSIP_param->stocrhs, fname);
        // Allocate array for row indices of the stochastic rhs entries
        DDSIP_data->rhsind = (int *) DDSIP_Alloc (sizeof (int), DDSIP_param->stocrhs, "rhsind(ReadData)");
        // Read the names of the contraints with stochastic right-hand sides and determine indices
        if (DDSIP_Find (datafile, "Names"))
        {
            for (j = 0; j < DDSIP_param->stocrhs; j++)
            {
                if((k = DDSIP_ReadWord (datafile, tmpdata, maxVarNameLength)))
                {
                    for (k = 0; k < (DDSIP_param->firstcon + DDSIP_param->seccon); k++)
                    {
                        if (!strcmp(tmpdata, rowname[k]))
                        {
                            DDSIP_data->rhsind[j] = k;
///////////////////////////
printf(" stoch rhs %d in row %d: '%s'\n", j, k,  rowname[k]);
///////////////////////////
                            break;
                        }
                    }
                    if (k ==  (DDSIP_param->firstcon + DDSIP_param->seccon))
                    {
                        printf ("XXX ERROR: row name '%s' specified in the stochastic rhs data file not found in the problem.\n", tmpdata);
                        fprintf (DDSIP_outfile, "XXX ERROR: row name '%s' specified in the stochastic rhs data file not found in the problem.\n", tmpdata);
                        exit(1);
                    }
                }
                else
                {
                    printf ("XXX ERROR: row name section of the stochastic rhs data file contains only %d out of %d names.\n", j, DDSIP_param->stocrhs);
                    fprintf (DDSIP_outfile, "XXX ERROR: row name section of the stochastic rhs data file contains only %d out of %d names.\n", j, DDSIP_param->stocrhs);
                    exit(1);
                }
            }
        }
        else
        {
            printf ("Cannot find section 'Names' in file '%s' (ReadData)\n", fname);
            return 1;
        }
    }
    else
        printf ("\t\t - probabilities from `%s'.\n", fname);

    probsum = 0.0;
    for (i = 0; i < DDSIP_param->scenarios; i++)
    {
        // Initialize
        sprintf (checkstr, "XX");
        // Read
        j = fscanf (datafile, "%s", checkstr);

        if (!(strncmp (checkstr, identifier, 2)))
        {
            j = fscanf (datafile, "%s", tmpdata);
            DDSIP_data->prob[i] = atof (tmpdata);
            probsum += DDSIP_data->prob[i];

            for (j = 0; j < DDSIP_param->stocrhs; j++)
            {
                k = fscanf (datafile, "%s", tmpdata);
                DDSIP_data->rhs[i * DDSIP_param->stocrhs + j] = atof (tmpdata);
            }
        }
        else
        {
            printf ("ERROR: Failed to read scenario keyword %d (%s)\n", i, checkstr);
            return 1;
        }

    }
    fprintf (DDSIP_outfile, "\nSTOCHASTIC RHS DATA READ FROM `%s': \n", fname);

    if (fabs (1. - probsum) > DDSIP_param->accuracy)
    {
        printf ("***Warning: Sum of probabilities %.15g differs from 1, scaling done.\n",probsum);
        fprintf (DDSIP_outfile, "***Warning: Sum of probabilities %.15g differs from 1, scaling done. New probabilities:\n",probsum);
        for (j = 0; j < DDSIP_param->scenarios; j++)
        {
            DDSIP_data->prob[j] = DDSIP_data->prob[j] / probsum;
            fprintf (DDSIP_outfile, "sc(%3d)=%g, ", j+1, DDSIP_data->prob[j]);
            if (!((j + 1) % 10))
                fprintf (DDSIP_outfile, "\n");
#ifdef NEOS
            /* sanity check:*/
            /* for the NEOS server it is an error to have probabilities nearly zero */
            if (DDSIP_data->prob[j] < 1e-12)
            {
                printf ("XXX ERROR: probability of scenario %d too small - exiting.\n",j);
                exit (111);
            }
#endif
        }
        if (j % 10)
        {
            fprintf (DDSIP_outfile, "\n");
        }
    }


    fclose (datafile);

    // Check if some scenarios are identical
    if (DDSIP_param->stocrhs)
    {
        status = 0;
        for (i = 0; i < DDSIP_param->scenarios; i++)
            for (j = i + 1; j < DDSIP_param->scenarios; j++)
            {
                k = 0;
                while (k < DDSIP_param->stocrhs && DDSIP_Equal (DDSIP_data->rhs[i * DDSIP_param->stocrhs + k], DDSIP_data->rhs[j * DDSIP_param->stocrhs + k]))
                    k++;

                if (k == DDSIP_param->stocrhs)
                {
                    status = 1;
                    fprintf (DDSIP_outfile, "Scenario %d and scenario %d are identical\n", i + 1, j + 1);
                }
            }

#ifndef NEOS
        if (status)
            printf ("*Warning: Identical scenarios.\n");
#else
        // for the NEOS server this is an error
        if (status)
        {
            printf ("XXX ERROR: Identical scenarios.\n");
            exit (1);
        }
#endif
    }

    if (DDSIP_param->files > 1)
    {
        sprintf (fname, "%s/rhs.out", DDSIP_outdir);
        if ((checkfile = fopen (fname, "w")) == NULL)
            printf ("*Warning: Failed to open '%s'\n.", fname);

        fprintf (checkfile, "probabilities:\n");
        for (i = 0; i < DDSIP_param->scenarios; i++)
        {
            if (!(i%10))
                fprintf (checkfile, "\n%6d |\t",i);
            fprintf (checkfile, "%8.4f\t", DDSIP_data->prob[i]);
        }
        fprintf (checkfile, "\n");

        for (j = 0; j < DDSIP_param->stocrhs; j++)
        {
            fprintf (checkfile, "rhs[%d]:",j);
            for (i = 0; i < DDSIP_param->scenarios; i++)
            {
                if(!(i%10))
                    fprintf (checkfile, "\n%6d |\t",i);
                fprintf (checkfile, "%8.4f\t", DDSIP_data->rhs[i * DDSIP_param->stocrhs + j]);
            }
            fprintf (checkfile, "\n");
        }
        fclose (checkfile);
    }
    // data->cost contains: stoch. costs for all scenarios, followed by the original costs
    DDSIP_data->cost =
        (double *) DDSIP_Alloc (sizeof (double),
                                DDSIP_param->scenarios * DDSIP_param->stoccost + DDSIP_param->firstvar + DDSIP_param->secvar, "cost (ReadData)");
    // Read stochastic costs if specified
    if (DDSIP_param->stoccost)
    {
        DDSIP_data->costind = (int *) DDSIP_Alloc (sizeof (int), DDSIP_param->stoccost, "costind(ReadData)");

        printf ("Enter data file name (stochastic cost):  ");
        k = scanf ("%s", fname);

        if (k == 0 || (datafile = fopen (fname, "r")) == NULL)
        {
            printf ("ERROR: Failed to open '%s' (ReadData)\n", fname);
            return 1;
        }

        printf ("\n\t\t - %d stochastic cost coefficients from `%s'.\n", DDSIP_param->stoccost, fname);

        // Read the names of the variables with stochastic cost coefficients and determine indices
        if (DDSIP_Find (datafile, "Names"))
        {
            for (j = 0; j < DDSIP_param->stoccost; j++)
            {
///////////////////////////
printf(" stoch cost names read: j = %d DDSIP_param->stoccost =  %d\n", j, DDSIP_param->stoccost);
///////////////////////////

                if((k = DDSIP_ReadWord (datafile, tmpdata, maxVarNameLength)))
                {
                    for (k = 0; k < (DDSIP_param->firstvar + DDSIP_param->secvar); k++)
                    {
                        if (!strcmp(tmpdata, colname[k]))
                        {
                            DDSIP_data->costind[j] = k;
///////////////////////////
printf(" stoch cost %d in variable %d: '%s'\n", j, k,  colname[k]);
///////////////////////////
                            break;
                        }
                    }
                    if (k ==  (DDSIP_param->firstvar + DDSIP_param->secvar))
                    {
                        printf ("XXX ERROR: column name '%s' specified in the stochastic cost data file not found in the problem.\n", tmpdata);
                        fprintf (DDSIP_outfile, "XXX ERROR: rowumn name '%s' specified in the stochastic cost data file not found in the problem.\n", tmpdata);
                        exit(1);
                    }
                }
                else
                {
                    printf ("XXX ERROR: columns name section of the stochastic cost data file contains only %d out of %d names.\n", j, DDSIP_param->stocrhs);
                    fprintf (DDSIP_outfile, "XXX ERROR: columns name section of the stochastic cost data file contains only %d out of %d names.\n", j, DDSIP_param->stocrhs);
                    exit(1);
                }
            }
        }
        else
        {
            printf ("Cannot find section 'Names' in file '%s' (ReadData)\n", fname);
            return 1;
        }
        for (i = 0; i < DDSIP_param->scenarios; i++)
        {
            // Initialize
            sprintf (checkstr, "XX");
            // Read
            k = fscanf (datafile, "%s", checkstr);
            if (!(strncmp (checkstr, identifier, 2)))
                for (j = 0; j < DDSIP_param->stoccost; j++)
                {
                    k = fscanf (datafile, "%s", tmpdata);
                    DDSIP_data->cost[i * DDSIP_param->stoccost + j] = atof (tmpdata);
                }
            else
            {
                printf ("ERROR: Failed to read scenarios keyword\n");
                return 1;
            }
        }

        fclose (datafile);
        fprintf (DDSIP_outfile, "\nSTOCHASTIC OBJECTIVE DATA READ FROM `%s': \n", fname);

        if (DDSIP_param->files > 1)
        {
            sprintf (fname, "%s/cost.out", DDSIP_outdir);
            if ((checkfile = fopen (fname, "w")) == NULL)
                printf ("*Warning: Failed to open '%s'\n.", fname);

            for (j = 0; j < DDSIP_param->stoccost; j++)
            {
                for (i = 0; i < DDSIP_param->scenarios; i++)
                    fprintf (checkfile, "%f\t", DDSIP_data->cost[i * DDSIP_param->stoccost + j]);
                fprintf (checkfile, "\n");
            }

            fclose (checkfile);
        }
    }
    // Store original objective coefficients
    i = CPXgetnumcols (DDSIP_env, DDSIP_lp);
    j = DDSIP_param->firstvar + DDSIP_param->secvar;
    if (i < j)
    {
        printf ("XXX Error: number of variables of read problem is %d, less than the total of %d specified in specs.\n",
                i, j);
        return (i - j);
    }
    status = CPXgetobj (DDSIP_env, DDSIP_lp, DDSIP_data->cost + DDSIP_param->scenarios * DDSIP_param->stoccost, 0, DDSIP_param->firstvar + DDSIP_param->secvar - 1);
    if (status)
    {
        printf ("ERROR: Failed to get objective coefficients\n");
        return status;
    }
    // Read stochastic matrix entries if specified
    if (DDSIP_param->stocmat)
    {
        DDSIP_data->matval = (double *) DDSIP_Alloc (sizeof (double), DDSIP_param->scenarios * DDSIP_param->stocmat, "matval(ReadData)");
        DDSIP_data->matcol = (int *) DDSIP_Alloc (sizeof (int), DDSIP_param->stocmat, "matcol(ReadData)");
        DDSIP_data->matrow = (int *) DDSIP_Alloc (sizeof (int), DDSIP_param->stocmat, "matrow(ReadData)");

        strcpy (identifier, "pos");

        printf ("Enter data file name (stochastic matrix entries):  ");
        k = scanf ("%s", fname);

        if (k == 0 || (datafile = fopen (fname, "r")) == NULL)
        {
            printf ("ERROR: Failed to open '%s' (ReadDate)\n", fname);
            return 1;
        }

        printf ("\n\t\t - %d stochastic matrix entries from `%s'.\n", DDSIP_param->stocmat, fname);

        // Initialize
        sprintf (checkstr, "XX");
        // Read
        // Read the names of the variables with stochastic matrix coefficients and determine indices
        if (DDSIP_Find (datafile, "Names"))
        {
            for (j = 0; j < DDSIP_param->stocmat; j++)
            {
                if((k = DDSIP_ReadWord (datafile, tmpdata, maxVarNameLength)))
                {
                    for (k = 0; k < (DDSIP_param->firstcon + DDSIP_param->seccon); k++)
                    {
                        if (!strcmp(tmpdata, rowname[k]))
                        {
                            DDSIP_data->matrow[j] = k;
///////////////////////////
printf(" stoch matrix %d in row %d: '%s'\n", j, k,  rowname[k]);
///////////////////////////
                            break;
                        }
                    }
                    if (k ==  (DDSIP_param->firstcon + DDSIP_param->seccon))
                    {
                        printf ("XXX ERROR: row name '%s' specified in the stochastic matrix data file not found in the problem.\n", tmpdata);
                        fprintf (DDSIP_outfile, "XXX ERROR: row name '%s' specified in the stochastic matrix data file not found in the problem.\n", tmpdata);
                        exit(1);
                    }
                }
                else
                {
                    printf ("XXX ERROR: name section of the stochastic matrix data file contains only %d out of %d names.\n", j, DDSIP_param->stocrhs);
                    fprintf (DDSIP_outfile, "XXX ERROR: name section of the stochastic matrix data file contains only %d out of %d names.\n", j, DDSIP_param->stocrhs);
                    exit(1);
                }
                if((k = DDSIP_ReadWord (datafile, tmpdata, maxVarNameLength)))
                {
                    for (k = 0; k < (DDSIP_param->firstvar + DDSIP_param->secvar); k++)
                    {
                        if (!strcmp(tmpdata, colname[k]))
                        {
                            DDSIP_data->matcol[j] = k;
///////////////////////////
printf(" stoch matrix %d in variable %d: '%s'\n", j, k,  colname[k]);
///////////////////////////
                            break;
                        }
                    }
                    if (k ==  (DDSIP_param->firstvar + DDSIP_param->secvar))
                    {
                        printf ("XXX ERROR: column name '%s' specified in the stochastic matrix data file not found in the problem.\n", tmpdata);
                        fprintf (DDSIP_outfile, "XXX ERROR: rowumn name '%s' specified in the stochastic matrix data file not found in the problem.\n", tmpdata);
                        exit(1);
                    }
                }
                else
                {
                    printf ("XXX ERROR: name section of the stochastic matrix data file contains only %d out of %d names.\n", j, DDSIP_param->stocrhs);
                    fprintf (DDSIP_outfile, "XXX ERROR: name section of the stochastic matrix data file contains only %d out of %d names.\n", j, DDSIP_param->stocrhs);
                    exit(1);
                }
            }
        }
        else
        {
            printf ("Cannot find section 'Names' in file '%s' (ReadData)\n", fname);
            return 1;
        }

        strcpy (identifier, "sce");

        for (i = 0; i < DDSIP_param->scenarios; i++)
        {
            // Initialize
            sprintf (checkstr, "XX");
            // Read
            k = fscanf (datafile, "%s", checkstr);

            if (!(strncmp (checkstr, identifier, 2)))
                for (j = 0; j < DDSIP_param->stocmat; j++)
                {
                    k = fscanf (datafile, "%s", tmpdata);
                    DDSIP_data->matval[i * DDSIP_param->stocmat + j] = atof (tmpdata);
                }
            else
            {
                printf ("ERROR: Failed to read scenarios keyword in %s \n", fname);
                return 1;
            }
        }

        fclose (datafile);
        fprintf (DDSIP_outfile, "\nSTOCHASTIC MATRIX DATA READ FROM `%s': \n", fname);

        if (DDSIP_param->files > 1)
        {
            sprintf (fname, "%s/matrix.out", DDSIP_outdir);
            if ((checkfile = fopen (fname, "w")) == NULL)
                printf ("*Warning: Failed to open '%s'\n.", fname);


            for (j = 0; j < DDSIP_param->stocmat; j++)
            {
                fprintf (checkfile, "%d %d\t", DDSIP_data->matrow[j], DDSIP_data->matcol[j]);
                for (i = 0; i < DDSIP_param->scenarios; i++)
                    fprintf (checkfile, "%f\t", DDSIP_data->matval[i * DDSIP_param->stocmat + j]);
                fprintf (checkfile, "\n");
            }

            fclose (checkfile);
        }
    }

    if (DDSIP_param->order)
    {
        int ind;
        DDSIP_data->order = (int *) DDSIP_Alloc (sizeof (int), DDSIP_param->firstvar, "order(ReadData)");

        printf ("Enter file name with priority order (master):  ");
        k = scanf ("%s", fname);

        if (k == 0 || (datafile = fopen (fname, "r")) == NULL)
        {
            printf ("\nERROR: Failed to open '%s' (ReadData)\n", fname);
            return 1;
        }

        printf ("\n\t Reading priority order from `%s'.\n", fname);

        for (i = 0; i < DDSIP_param->firstvar; i++)
        {
            DDSIP_data->order[i] = 1;
        }
        for (i = 0; i < DDSIP_param->firstvar; i++)
        {
            if (fscanf (datafile, "%d", &ind) == EOF)
                break;
            if (ind < 0 || ind >= DDSIP_param->firstvar)
            {
                printf ("ERROR: Wrong DDSIP_index (%d) in priority order file.\n", ind);
                return 1;
            }
            if (fscanf (datafile, "%d", &DDSIP_data->order[ind]) < 1)
                break;
        }
        fclose (datafile);

        if (DDSIP_param->files)
        {
            sprintf (fname, "%s/order.out", DDSIP_outdir);
            if ((checkfile = fopen (fname, "w")) == NULL)
                printf ("*Warning: Failed to open '%s'\n.", fname);

            for (i = 0; i < DDSIP_param->firstvar; i++)
                fprintf (checkfile, "%d \t %d\n", i + 1, DDSIP_data->order[i]);
            fclose (checkfile);
        }
    }


    DDSIP_Free ((void **) &(colstore));
    DDSIP_Free ((void **) &(colname));
    DDSIP_Free ((void **) &(rowstore));
    DDSIP_Free ((void **) &(rowname));
    return 0;
}

//==========================================================================
// If there are advanced starting infos provided -> read them
// We trust that the data is consistent.
int
DDSIP_AdvStart (void)
{
    char fname[DDSIP_ln_fname];
    FILE *advfile;
    int i, k;
    double tmp;

    printf ("Enter file name for start info: ");
    k = scanf ("%s", fname);

    if (k == 0 || (advfile = fopen (fname, "r")) == NULL)
    {
        printf ("ERROR: Cannot open '%s' (AdvStart) \n", fname);
        return 1;
    }

    printf ("\n\t Reading start info from `%s'.\n", fname);

    fprintf (DDSIP_outfile, "-START INFORMATION\n");
    if (DDSIP_Find (advfile, "BEST"))
    {
        k = fscanf (advfile, "%lf", &tmp);
        fprintf (DDSIP_outfile, " BEST VALUE  %13.7f\n", tmp);
        printf ("\t  Best %f\n", tmp);
        DDSIP_bb->bestvalue = DDSIP_Dmin (DDSIP_bb->bestvalue, tmp);
    }
    if (DDSIP_Find (advfile, "BOUND"))
    {
        k = fscanf (advfile, "%lf", &DDSIP_node[0]->bound);
        fprintf (DDSIP_outfile, " BOUND       %13.7f\n", DDSIP_node[0]->bound);
        printf ("\t  Bound %f\n", DDSIP_node[0]->bound);
    }

    if (DDSIP_Find (advfile, "SOLUTI"))
    {
        fprintf (DDSIP_outfile, " SOLUTION\n");
        // This setting indicates that a solution has been provided
        DDSIP_param->advstart = 2;
        DDSIP_bb->adv_sol = (double *) DDSIP_Alloc (sizeof (double), DDSIP_bb->firstvar, "DDSIP_bb->adv_sol(sipread)");
        for (i = 0; i < DDSIP_param->firstvar; i++)
        {
            k = fscanf (advfile, "%lf", DDSIP_bb->adv_sol + i);
//      fprintf (DDSIP_outfile, "  %14.8g", DDSIP_bb->adv_sol[i]);
//      if (!((i+1)%10))
//        fprintf (DDSIP_outfile, "\n");
        }
    }

    if (DDSIP_Find (advfile, "MULTIP"))
    {
        fprintf (DDSIP_outfile, " MULTIPLIER\n");
        for (i = 0; i < DDSIP_bb->dimdual; i++)
        {
            k = fscanf (advfile, "%lf", &DDSIP_node[0]->dual[i]);
//      fprintf (DDSIP_outfile, "  %14.8g", DDSIP_node[0]->dual[i]);
//      if (!((i+1)%10))
//        fprintf (DDSIP_outfile, "\n");
        }
//    fprintf (DDSIP_outfile, "\n");
    }
    fclose (advfile);

    return 0;
}