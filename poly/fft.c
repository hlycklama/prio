
#include "fft.h"
#include "util.h"

static void
fft_recurse(fmpz_t *out, const fmpz_t mod, int n, fmpz_t *roots, fmpz_t *ys,
    fmpz_t *tmp, fmpz_t *ySub, fmpz_t *rootsSub)
{
  if (n == 1) {
    fmpz_set(out[0], ys[0]);
    return;
  }

  // Recurse on the first half 
  for (int i=0; i<n/2; i++) {
    fmpz_add(ySub[i], ys[i], ys[i+(n/2)]);
    fmpz_mod(ySub[i], ySub[i], mod);

    fmpz_set(rootsSub[i], roots[2*i]);
  }

  fft_recurse(tmp, mod, n/2, rootsSub, ySub, &tmp[n/2], &ySub[n/2], &rootsSub[n/2]);
  for (int i=0; i<n/2; i++) {
    fmpz_set(out[2*i], tmp[i]);
  }

  // Recurse on the second half 
  for (int i=0; i<n/2; i++) {
    fmpz_sub(ySub[i], ys[i], ys[i+(n/2)]);
    fmpz_mod(ySub[i], ySub[i], mod);
    fmpz_mul(ySub[i], ySub[i], roots[i]);
    fmpz_mod(ySub[i], ySub[i], mod);
  }

  fft_recurse(tmp, mod, n/2, rootsSub, ySub, &tmp[n/2], &ySub[n/2], &rootsSub[n/2]);
  for (int i=0; i<n/2; i++) {
    fmpz_set(out[2*i + 1], tmp[i]);
  }
}

char *fft_interpolate(char *modIn, int nPoints, 
    char **rootsIn, char **pointsYin, bool invert)
{

  fmpz_t mod;
  fmpz_init_from_gostr(mod, modIn);

  fmpz_t *ys = (fmpz_t*)malloc(nPoints*sizeof(fmpz));
  fmpz_t *roots = (fmpz_t*)malloc(nPoints*sizeof(fmpz));
  for (int i=0; i<nPoints; i++) {
    fmpz_init_from_gostr(roots[i], rootsIn[i]);
    fmpz_init_from_gostr(ys[i], pointsYin[i]);
  }

  printf("Hey init\n");
    
  fmpz_t *out = (fmpz_t*)malloc(nPoints*sizeof(fmpz));
  fmpz_t *tmp = (fmpz_t*)malloc(nPoints*sizeof(fmpz));
  fmpz_t *ySub = (fmpz_t*)malloc(nPoints*sizeof(fmpz));
  fmpz_t *rootsSub = (fmpz_t*)malloc(nPoints*sizeof(fmpz));
  for (int i=0; i<nPoints;i++) {
    fmpz_init(out[i]);
    fmpz_init(tmp[i]);
    fmpz_init(ySub[i]);
    fmpz_init(rootsSub[i]);
  }

  fft_recurse(out, mod, nPoints, roots, ys,
      tmp, ySub, rootsSub);

  if (invert) {
    fmpz_t n_inverse;
    fmpz_init(n_inverse);
    fmpz_set_ui(n_inverse, nPoints);
    fmpz_invmod(n_inverse, n_inverse, mod);
    for (int i=0; i<nPoints;i++) {
      fmpz_mul(out[i], out[i], n_inverse);
      fmpz_mod(out[i], out[i], mod);
    }
    fmpz_clear(n_inverse); 
  }

  for (int i=0; i<nPoints;i++) {
    fmpz_clear(ys[i]);
    fmpz_clear(roots[i]);

    fmpz_clear(tmp[i]);
    fmpz_clear(ySub[i]);
    fmpz_clear(rootsSub[i]);
  }

  free(ys);
  free(roots);

  free(tmp);
  free(ySub);
  free(rootsSub);

  return fmpz_array_to_str(nPoints, out);
}  

