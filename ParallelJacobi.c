/* Parallel Jacobi Relaxation Program */

#include <stdlib.h>
#include <math.h>

#define n 64   /* Array Size */
#define tolerance .01 /* Tolerance Value */

int stream upper[n + 1], stream lower[n + 1]; /* Stream Variables */
int i, j, k, global;
float update, max;
float  x[n + 2][n + 2], y[n + 2][n + 2];
boolean global_done;

spinlock arrival, departure;

void Barrier(int i)  /* Barrier Function */
{
    int variable;
      if (i > 1)
      {
       send(upper[i - 1], 1); /* Send Signal */
      }
    if(i < n) {
        send(lower[i + 1], 1);    /* Send Signal */
        recv(upper[i], variable);    /* Receive Signal */
    }
    if (i > 1)
    {
        recv(lower[i], variable);    /* Receive Signal */
    }
    }
    boolean Aggregate(boolean completed){   /* Aggregate Function */
        
        boolean result;
        
        Lock(arrival);  /* Lock */
        
        global = global + 1;
        global_done = global_done && completed;
        
        if (global < n)
        {
            Unlock(arrival);    /* Unlock */
        }
        else
        {
            Unlock(departure);  /* Unlock */
        }
        Lock(departure);    /* Lock */
        
        global = global - 1;
        result = global_done;
        
        if (global > 0)
        {
            Unlock(departure);  /* Unlock */
        }
        else {
            Unlock(arrival);    /* Unlock */
            global_done = true;
        }
        return(result);
}

main()
{
    global = 0;
    
    Unlock(arrival);    /* Unlock */
    Lock(departure);    /* Lock */
    
    global_done = true;
    
    forall i = 0 to n + 1 grouping 10 do {
        forall j = 0 to n + 1 do
            x[i][j] = (rand() % 10000) / 100.0;
    }

    y = x;

    forall i = 1 to n do { 
        
        int j;
        boolean complete_bool;
        float update, max;
        
        do  {
            max = 0;
            for(j = 1; j <= n; j++)
            {
                /* Calculate edge values */
                y[i][j] = (x[i - 1][j] + 
                                            x[i + 1][j] + 
                                            x[i][j - 1] + 
                                            x[i][j + 1]) / 4;
                
                update = fabs(y[i][j] - x[i][j]);
                
                if (update > max) /* Compare max */
                {
                    max = update;
                }
            }
            Barrier(i); /* Barrier Function */
            x[i] = y[i];
            Barrier(i); /* Barrier Function */
            
            complete_bool = Aggregate(max < tolerance);  /* Aggreagte Function */
        }
        while (!complete_bool);
    }
    
    cout.precision(4);  /* Set Precision */
    /* Print 2D array */
    for(i = 0; i <= n + 1; i++)
    {
        cout<< "Row" << i << ":" << endl;    /* Print Row Number */
        for(j = 0; j <= n+1; j++)
        {
            if((j > 0) && (j % 5) == 0){
                cout << endl;
            }
            cout << x[i][j] << " ";    /* Print columns */
        }
        cout << endl;
    }
}