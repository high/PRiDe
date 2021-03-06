#include "Propagate.h"
#include "Package.h"
#include "Config.h"
#include "Debug.h"
#include "Network.h"
#include "DBoid.h"

#include <math.h>

void propagateList( GSList *methodCalls, GSList *replicas, dboid_t dboid )
{
	Propagation2Package *ppack;
	int it, it2;
	int psize;
	int start, end;
	int numCalls;
	int numberPackages; 	/* The number of packages that will be sent */
	int packSize;			/* The numeber of updates in a package */
	int updatesLeft;		/* Stores the number of updates that is left to send */
	int updatesInPackage;	
	void *data;
	
	packSize = PROP_PACK_SIZE;
	
	numCalls = g_slist_length( methodCalls );
	updatesLeft = numCalls;
	
	/* Calucales the number of packages that is needed to send the data */
	numberPackages = ceil( numCalls / (double)packSize );
	
	__DEBUG( "Number of propagation packages to send: %d for %d updates", numberPackages, numCalls );
	
	for( it = 0; it < numberPackages; it++ )
	{
		if(updatesLeft > packSize )
		{
			updatesInPackage = packSize;
		}
		else
		{
			updatesInPackage = updatesLeft;
		}
		
		/* Create memory for the package + the memory for the updates in the package  */
		psize = sizeof(Propagation2Package) + ( sizeof( MethodCallObject ) * updatesInPackage );
		ppack = malloc( psize );
		
		/* Set package variables */
		ppack->size = psize;
		ppack->pack_type = PACK_PROP2;
		ppack->replica_id = __conf.id;
		dboidCopy( ppack->dboid, dboid, sizeof( ppack->dboid ) );
		ppack->numberOfMethodCalls = updatesInPackage;
		
		/* Populates the package with the updates */
		for( it2 = 0; it2 < updatesInPackage; it2++ )
		{
			/* Get the first update */
			data = methodCalls->data;
			
			/* Store in the package */
			ppack->objects[it2] = * (MethodCallObject *) data;
			
			if( it2 == 0 )
			{
				start = ((MethodCallObject *) data)->generationNumber;
			}

			end = ((MethodCallObject *) data)->generationNumber;
			/* 
			* Remove the update from the list 
			* This should be equal to using a pop function on a general container structure
			*/
			methodCalls = g_slist_remove( methodCalls, data);
		}
		
		__DEBUG( "Propagating %d methods with gen range{%d, %d} ", updatesInPackage, start, end );
		
		/* Sénd package to all replicas */
		networkSendDataToAll( replicas, ppack, ppack->size );
		
		/* Remove all data for the package */
		free( ppack );
		
		/* Remove the updates that have been send */
		updatesLeft = updatesLeft - updatesInPackage;
	}
}
