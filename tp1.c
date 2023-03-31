#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]);
void timestamp();

int main(int argc, char *argv[])

{
  int *a;
  int *element;
  int ans;
  int *b;
  int dest;
  int dummy;
  int i;
  int ierr;
  int k;
  int m;
  int master = 0;
  int my_id;
  int num_procs;
  int num_rows;
  int num_workers;
  MPI_Status status;
  int tag;
  int tag_done;
  int *escalar;

  ierr = MPI_Init(&argc, &argv);

  if (ierr != 0)
  {
    printf("\n");
    printf("MATVEC_MPI - Fatal error!\n");
    printf("  MPI_Init returns nonzero IERR.\n");
    exit(1);
  }

  ierr = MPI_Comm_rank(MPI_COMM_WORLD, &my_id);

  ierr = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  if (my_id == 0)
  {
    timestamp();
    printf("\n");
    printf("  An MPI ejemplo\n");
    printf("  producto entre un escalar p y una matriz A \n");
    printf("\n");
    printf("  Compiled on %s at %s.\n", __DATE__, __TIME__);
    printf("\n");
    printf("  The number of processes is %d.\n", num_procs);
  }
  printf("\n");
  printf("Process %d is active.\n", my_id);

  m = 20;
  tag_done = m + 1;

  if (my_id == 0)
  {
    printf("\n");
    printf("  The number of rows is    %d.\n", m);
  }
  if (my_id == master)
  {
    a = (int *)malloc(m * sizeof(int));
    escalar = (int *)malloc(sizeof(int));
    b = (int *)malloc(m * sizeof(int));

    k = 0;
    for (i = 1; i <= m; i++)
    {
      a[k] = k + 1;
      k = k + 1;
    }

    // Se inicializa el escalar
    *escalar = 2.0;

    printf("\n");
    printf("Theo - Proceso maestro:\n");
    printf("Vector A:\n");
    printf("\n");
    for (i = 0; i < m; i++)
    {
      printf("%d %d\n", i, a[i]);
    }
    printf("\nEscalar: %d\n", *escalar);
  }
  else
  {
    // b = (int *)malloc(m * sizeof(int));
    element = (int *)malloc(sizeof(int));
    escalar = (int *)malloc(sizeof(int));
  }
  ierr = MPI_Bcast(escalar, 1, MPI_INT, master, MPI_COMM_WORLD);

  if (my_id == master)
  {
    num_rows = 0;
    for (i = 1; i <= num_procs - 1; i++)
    {
      dest = i;
      tag = num_rows;
      ierr = MPI_Send(a + tag, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
      num_rows = num_rows + 1;
    }

    num_workers = num_procs - 1;

    for (;;)
    {
      ierr = MPI_Recv(&ans, 1, MPI_INT, MPI_ANY_SOURCE,
                      MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      tag = status.MPI_TAG;

      b[tag] = ans;

      if (num_rows < m)
      {
        dest = status.MPI_SOURCE;
        tag = num_rows;
        k = num_rows;
        ierr = MPI_Send(a + tag, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
        num_rows = num_rows + 1;
      }
      else
      {
        num_workers = num_workers - 1;
        dummy = 0;
        dest = status.MPI_SOURCE;
        tag = tag_done;

        ierr = MPI_Send(&dummy, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
        if (num_workers == 0)
        {
          break;
        }
      }
    }

    printf("Fin\n");

    free(a);
    printf("Free a\n");
    free(escalar);
    printf("Free escalar\n");
  }

  else
  {
    for (;;)
    {
      ierr = MPI_Recv(element, 1, MPI_INT, master, MPI_ANY_TAG,
                      MPI_COMM_WORLD, &status);

      tag = status.MPI_TAG;

      if (tag == tag_done)
      {
        printf("Process %d shutting down.\n", my_id);
        break;
      }
      ans = *element * *escalar;
      // printf("Proceso: %d   Elemento: %d  Escalar:%d  Resultado: %d \n", my_id, *element, *escalar, ans);
      ierr = MPI_Send(&ans, 1, MPI_INT, master, tag, MPI_COMM_WORLD);
    }

    free(element);
    free(escalar);
  }

  if (my_id == master)
  {
    printf("\n");
    printf("Theo - Proceso master:\n");
    printf("Product del escalar escalar * A = b\n");
    printf("\n");
    for (i = 0; i < m; i++)
    {
      printf("%d %d\n", i, b[i]);
    }

    free(b);
  }
  /*
    Terminate MPI.
  */
  ierr = MPI_Finalize();
  /*
    Terminate.
  */
  if (my_id == master)
  {
    printf("\n");
    printf("MATVEC - Master process:\n");
    printf("  Normal end of execution.\n");
    printf("\n");
    timestamp();
  }
  return 0;
}

void timestamp()

{
#define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct tm *tm;
  time_t now;

  now = time(NULL);
  tm = localtime(&now);

  strftime(time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm);

  printf("%s\n", time_buffer);

  return;
#undef TIME_SIZE
}