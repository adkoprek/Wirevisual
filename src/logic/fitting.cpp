#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlin.h>
#define True                                    (1==1)
#define False                                   !True

#define sign(x) (((x) < 0) ? (-1) : (1))

/// only for emittance fit
static double      bro, qlen, drift;
static int         emittype;

void setEmitData(int emitType, float Bro, float Qlen, float Drift) {
  emittype = emitType;
  bro = Bro;
  qlen = Qlen;
  drift = Drift;
}

void
print_fitstate(size_t iter, gsl_multifit_fdfsolver *s);
struct data {
  size_t          n;
  size_t          ft;
  double          *x;
  double          *y;
  double          *sigma;
};

double fchisq (int npts, float *y, float *yfit, int freedom)
/*
  C       PURPOSE
  C        EVALUATE REDUCED CHI SQUARE FOR FIT TO DATA
  C        FCHISQ = SUM((Y - YFIT)**2 / SIGMA**2)/NFREE
  C
  C       PARAMETERS
  C         Y       - ARRAY OF DATA POINTS
  C         NPTS    - NUMBER OF DATA POINTS
  C         NFREE   - DEGREES OF FREEDOM
  C         YFIT    - ARRAY OF CALCULATED VALUES OF Y
  C
*/
{
  int i;
  double chisq = 0.0;
  if(freedom <= 0) return 0.0;
  for(i=0; i < npts; i++) {
    chisq = chisq + pow(y[i] - yfit[i], 2);
  }
  return (double) (chisq/freedom);
}

/* function to fit */

void matEmitt(double t, double *r11, double *r12)
{
  double sinXoverX;
  double tc = sign(t) * qlen * sqrt(fabs(t)/bro);    /* normalize */

  if(emittype == 0) { /* horfocus-horprofile */

    if (tc < 1.e-6) sinXoverX = 1; else sinXoverX= sin(tc) / tc;
    *r12 = drift * cos(tc) + qlen * sinXoverX;
    if (tc > 0) {
      *r11 = cos(tc) - (drift/qlen) * tc * sin(tc);
    } else {
      *r11 = cos(tc) + (drift/qlen) * tc * sin(tc);
    }

  } else if(emittype == 1)  {  /* verfocus-verprofile   */

    if (tc < 1.e-6) sinXoverX = 1; else sinXoverX= sin(tc) / tc;
    *r12 = drift * cos(tc) + qlen * sinXoverX;
    if (tc > 0) {
      *r11 = cos(tc) + (drift/qlen) * tc * sin(tc);
    } else {
      *r11 = cos(tc) - (drift/qlen) * tc * sin(tc);
    }

  } else if(emittype == 2)  {  /* horfocus-verprofile */

    if (tc < 1.e-6) sinXoverX = 1; else sinXoverX= sinh(tc) / tc;
    *r12 = drift * cosh(tc) + qlen * sinXoverX;
    if (tc > 0) {
      *r11 = cosh(tc) + (drift/qlen) * tc * sinh(tc);
    } else {
      *r11 = cosh(tc) - (drift/qlen) * tc * sinh(tc);
    }

  } else if(emittype == 3)  {  /* verfocus-horprofile */

    if (tc < 1.e-6) sinXoverX = 1; else sinXoverX= sinh(tc) / tc;
    *r12 = drift * cosh(tc) + qlen * sinXoverX;
    if (tc > 0) {
      *r11 = cosh(tc) - (drift/qlen) * tc * sinh(tc);
    } else {
      *r11 = cosh(tc) + (drift/qlen) * tc * sinh(tc);
    }

  }

}

int
fit_f(const gsl_vector *x, void *params, gsl_vector *f)
{
  size_t          n = ((struct data *) params)->n;
  size_t          ft = ((struct data *) params)->ft;
  double          *xi = ((struct data *) params)->x;
  double          *y = ((struct data *) params)->y;
  double          *sigma = ((struct data *) params)->sigma;
  size_t          i;
  double          Yi;

  for (i = 0; i < n; i++) {
    double  t = xi[i];

    switch (ft) {
    case 0: {      /* linear function */
      //printf("linear\n");
      Yi = gsl_vector_get(x, 0) * t + gsl_vector_get(x, 1);
    }
      break;
    case 1: {      /* parabolic function */
      //printf("parabolic\n");
      Yi = gsl_vector_get(x, 0) * pow(t,2) +gsl_vector_get(x, 1) * t + gsl_vector_get(x, 2);
    }
      break;
    case 2: {      /* gaussian function */
      //printf("gauss 1\n");
      Yi = gsl_vector_get(x, 3) + gsl_vector_get(x, 0) * exp (-0.5 * pow(t- gsl_vector_get(x, 1), 2) /
                                                              pow(gsl_vector_get(x, 2), 2));
    }
      break;
    case 7: {      /* gaussian function */
      //printf("gauss 2\n");
      Yi = gsl_vector_get(x, 0) * exp (-0.5 * pow(t- gsl_vector_get(x, 1), 2) /
                                       pow(gsl_vector_get(x, 2), 2));
    }
      break;

    case 3: {      /* exponential function */
      //printf("exponential\n");
      Yi = gsl_vector_get(x, 0) * exp (-t * gsl_vector_get(x, 1)) +
        gsl_vector_get(x, 2);
    }
      break;
    case 4: {      /* log function */
      //printf("log\n");
      Yi = gsl_vector_get(x, 0) * log (t) +  gsl_vector_get(x, 1);
    }
      break;
    case 5: {     /* power function */
      //printf("power\n");
      Yi = gsl_vector_get(x, 0) * pow (t,gsl_vector_get(x, 1) ) +  gsl_vector_get(x, 2);
    }
      break;
    case 6: {     /* emittanz */
      //printf("emittanz\n");
      double r11, r12;
      matEmitt(t, &r11, &r12);
      Yi = gsl_vector_get(x, 0) * pow(r11, 2) + 2 * gsl_vector_get(x, 1) * r11 * r12 +
        gsl_vector_get(x, 2) * pow(r12, 2);
      Yi = 1.e6 * Yi;
      break;
    } default: {
      printf("fitting case unknown\n");
      exit(1);
    }
    }
    gsl_vector_set(f, i, (Yi - y[i]) / sigma[i]);
    fflush(stdout);
  }
  return GSL_SUCCESS;
}

/* differential of function to fit */

int
fit_df(const gsl_vector *x, void *params, gsl_matrix *J)
{
  size_t          n = ((struct data *) params)->n;
  size_t          ft = ((struct data *) params)->ft;
  double          *sigma = ((struct data *) params)->sigma;
  double          *xi = ((struct data *) params)->x;
  size_t          i;

  for (i = 0; i < n; i++) {
    double          t = xi[i];
    double          s = sigma[i];

    switch (ft) {
    case 0: {
      gsl_matrix_set(J, i, 0, t / s);
      gsl_matrix_set(J, i, 1, 1 / s);
    }
      break;
    case 1: {
      gsl_matrix_set(J, i, 0, pow(t,2) / s);
      gsl_matrix_set(J, i, 1, t / s);
      gsl_matrix_set(J, i, 2, 1 / s);
    }
      break;
    case 2:
      gsl_matrix_set (J, i, 3, 1 / s);
      // fall through
    case 7: {
      double c0 = gsl_vector_get(x, 0);
      double c1 = gsl_vector_get(x, 1);
      double c2 = gsl_vector_get(x, 2);
      double e =  exp (-0.5 * pow(t-gsl_vector_get(x, 1), 2) /
                       pow(gsl_vector_get(x, 2),2) );
      gsl_matrix_set (J, i, 0, e / s);
      gsl_matrix_set (J, i, 1, (c0 * (t-c1) * e) / pow(c2,2) / s);
      gsl_matrix_set (J, i, 2, (c0 * e * pow(t-c1,2)) / pow(c2,3) / s);
    }
      break;
    case 3: {
      double c0 = gsl_vector_get(x, 0);
      double c1 = gsl_vector_get(x, 1);
      double e =  exp (-t * c1);
      gsl_matrix_set (J, i, 0, e / s);
      gsl_matrix_set (J, i, 1, -t * c0 * e / s);
      gsl_matrix_set (J, i, 2, 1 / s);
    }
      break;
    case 4: {
      gsl_matrix_set (J, i, 0, log(t) / s);
      gsl_matrix_set (J, i, 1, 1 / s);
    }
      break;
    case 5: {
      double c0 = gsl_vector_get(x, 0);
      double c1 = gsl_vector_get(x, 1);
      double e =  pow(t, c1);
      gsl_matrix_set (J, i, 0, e / s);
      gsl_matrix_set (J, i, 1, c0 * log(t) * e / s);
      gsl_matrix_set (J, i, 2, 1 / s);
    }
      break;
    case 6: {
      double r11, r12;
      matEmitt(t, &r11, &r12);
      gsl_matrix_set(J, i, 0, pow(r11, 2) / s * 1.e6);
      gsl_matrix_set(J, i, 1, 2 * r11 * r12 / s * 1.e6);
      gsl_matrix_set(J, i, 2, pow(r12, 2) / s * 1.e6);
      break;
    }
    }

  }
  return GSL_SUCCESS;
}

int
fit_fdf(const gsl_vector *x, void *params,
        gsl_vector *f, gsl_matrix *J)
{
  fit_f(x, params, f);
  fit_df(x, params, J);
  return GSL_SUCCESS;
}

int
FunctionFit(int fittype, float *XP, float *RP, int Nvals, float *estimation,
            float *result, float *error, float *RPout, float *chisqr, int *Nparam, char *fiterr)
{
  const           gsl_multifit_fdfsolver_type *T;
  gsl_multifit_fdfsolver *s;
  int             status;
  size_t          i, iter = 0;
  /*
    printf("number of values=%d,  function=%d\n", Nvals, fittype);
  */
  if(fittype > 7) return False;

  int fitting = fittype;
  switch (fitting) {
  case 0:
    *Nparam = 2;
    break;
  case 1:
    *Nparam = 3;
    break;
  case 2:
    *Nparam = 4;
    break;
  case 7:
    *Nparam = 3;
    break;
  case 3:
    *Nparam = 3;
    break;
  case 4:
    *Nparam = 2;
    break;
  case 5:
    *Nparam = 3;
    break;
  case 6:
    *Nparam = 3;
  }

  const           size_t n = Nvals;
  const           size_t p = *Nparam;
  const           size_t ft = fitting;
  double          *xi, *y, *sigma, *x_init;
  gsl_matrix      *covar = gsl_matrix_alloc(p, p);

  xi = (double *) malloc(Nvals *sizeof(double));
  y = (double *) malloc(Nvals *sizeof(double));
  sigma = (double *) malloc(Nvals *sizeof(double));

  struct data d = {n, ft, xi, y, sigma};

  gsl_multifit_function_fdf f;

  x_init = (double *) malloc(*Nparam *sizeof(double));
  for (i = 0; i < p; i++) {
    x_init[i] = estimation[i];
  }
  gsl_vector_view x = gsl_vector_view_array(x_init, p);
  const           gsl_rng_type *type;
  gsl_rng         *r;

  gsl_rng_env_setup();
  type = gsl_rng_default;
  r = gsl_rng_alloc(type);
  f.f = &fit_f;
  f.df = &fit_df;
  f.fdf = &fit_fdf;
  f.n = n;
  f.p = p;
  f.params = &d;

  /* This is the data to be fitted */

  for (i = 0; i < n; i++) {
    xi[i] = XP[i];
    y[i] = RP[i];
    sigma[i] = 1.0;
  }
  T = gsl_multifit_fdfsolver_lmsder;
  s = gsl_multifit_fdfsolver_alloc(T, n, p);
  gsl_multifit_fdfsolver_set(s, &f, &x.vector);
  /*
    print_fitstate(iter, s);
  */
  do
    {
      iter++;

      status = gsl_multifit_fdfsolver_iterate(s);
      /*
        printf("status = %s\n", gsl_strerror(status));
      */
      strcpy(fiterr, gsl_strerror(status));

      //print_fitstate(iter, s);

      if (status) break;
      status = gsl_multifit_test_delta(s->dx, s->x,  1e-15, 1e-15);
    }
  while (status == GSL_CONTINUE && iter < 200);


  //if GSL_VERSION < 2
  //  gsl_multifit_covar(s->J, 0.0, covar);
  //
  //else // migration to GSL 2 from https://github.com/SciRuby/rb-gsl/commit/fbf775955495fc31b1b0a569f0aa40033c65d773
  gsl_matrix *J = NULL;
  J = gsl_matrix_alloc(n, p);
  gsl_multifit_fdfsolver_jac(s, J);
  gsl_multifit_covar(J, 0.0, covar);

  /*
    gsl_matrix_fprintf(stdout, covar, "%g");
  */

#define     FIT(i)   gsl_vector_get(s->x, i)
#define     ERR(i)   sqrt(gsl_matrix_get(covar,i,i))

  for (i = 0; i < p; i++) {
    result[i] = FIT(i);
    error[i] = ERR(i);
  }
  /*
    printf("fit params %lf+/-%lf %lf+/-%lf %lf+/-%lf\n", FIT(0), ERR(0),
    FIT(1), ERR(1),
    FIT(2), ERR(2));
  */
  strcpy(fiterr, gsl_strerror(status));
  /*
         printf("status = %s\n", gsl_strerror(status));
         printf("number of iterations=%d\n", iter);
  */
  gsl_multifit_fdfsolver_free(s);
  gsl_rng_free(r);
  gsl_matrix_free(covar);
  gsl_matrix_free(J);

  for (i = 0; i < n; i++) {
    double          t = xi[i];

    switch (ft) {
    case 0:
      RPout[i] = result[0] * t + result[1];
      break;
    case 1:
      RPout[i] = result[0] * pow(t,2) + result[1] * t + result[2];
      break;
    case 2:
      RPout[i] = result[3] + result[0] * exp (-0.5 * pow(t-result[1],2) / pow(result[2],2));
      break;
    case 7:
      RPout[i] = result[0] * exp (-0.5 * pow(t-result[1],2) / pow(result[2],2));
      break;
    case 3:
      RPout[i] = result[0] * exp (-t * result[1]) + result[2];
      break;
    case 4:
      RPout[i] = result[0] * log (t) + result[1];
      break;
    case 5:
      RPout[i] = result[0] * pow (t,result[1]) +  result[2];
      break;
    case 6: {
      double r11, r12;
      matEmitt(t, &r11, &r12);
      RPout[i] = result[0] * pow(r11, 2) + 2 * result[1] * r11 * r12 +
        result[2] * pow(r12, 2);
      RPout[i] = 1.e6 * RPout[i];
      break;
    }
    }
  }
  *chisqr =  fchisq (n, RP, RPout, n - *Nparam);

  free(sigma);
  free(y);
  free(xi);
  free(x_init);

  if (strstr(gsl_strerror(status), "success") != (char *)0) {
    return True;
  } else {
    return False;
  }
}

void
print_fitstate(size_t iter, gsl_multifit_fdfsolver *s)
{
  printf("iter: %3lu  %15.8f %15.8f %15.8f"
         "|f(x)| = %g\n",
         iter,
         gsl_vector_get(s->x, 0), gsl_vector_get(s->x, 1), gsl_vector_get(s->x, 2),
         gsl_blas_dnrm2(s->f));
}
