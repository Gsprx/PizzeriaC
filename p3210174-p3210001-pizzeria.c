#include "p3210174-p3210001-pizzeria.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>


unsigned int seedp; //global rand_r function seed
int g_clock = 0; //global clock for prints
int Ncust;	
int threads_finished;

//sales statistics variables declaration
int plain_pizzas = 0;
int special_pizzas = 0;
int income = 0;
int successful_orders = 0;
double max_order_time = 0;
double total_order_time = 0;
double max_cold_time = 0;
double total_cold_time = 0;

//order production variables declaration
int avail_cooks = Ncook;
int avail_ovens = Noven;
int avail_packers = Npacker;
//order delivery variables declaration
int avail_deliverers = Ndeliverer;
//mutex declarations
pthread_mutex_t statistics_lock;
pthread_mutex_t print_lock;
pthread_mutex_t gclock_lock;
pthread_mutex_t cook_lock;
pthread_mutex_t oven_lock;
pthread_mutex_t packer_lock;
pthread_mutex_t deliverer_lock;
//condition variables declaration
pthread_cond_t cook_cond;
pthread_cond_t oven_cond;
pthread_cond_t packer_cond;
pthread_cond_t deliverer_cond;

int main(int argc, char** argv){
	int i,j;

	//check for valid Ncust amount
	if (argc != 3) {
		printf("Invalid arguements.\n");
		return -1;
	}
	else {
		Ncust = atoi(argv[1]);
		seedp = atoi(argv[2]);
	}
	//initialize all mutex locks
	if (pthread_mutex_init(&print_lock, NULL) != 0){
		printf("Print mutex initialization has failed!\n");
		return -1;
	}
	if (pthread_mutex_init(&gclock_lock, NULL) != 0){
		printf("Global clock mutex initialization has failed!\n");
		return -1;
	}
	if (pthread_mutex_init(&gclock_lock, NULL) != 0){
		printf("Sales mutex initialization has failed!\n");
		return -1;
	}
	if (pthread_mutex_init(&cook_lock, NULL) != 0){
		printf("Producer mutex initialization has failed!\n");
		return -1;
	}
	if (pthread_mutex_init(&oven_lock, NULL) != 0){
		printf("Oven mutex initialization has failed!\n");
		return -1;
	}
	if (pthread_mutex_init(&packer_lock, NULL) != 0){
		printf("Packer mutex initialization has failed!\n");
		return -1;
	}
	if (pthread_mutex_init(&deliverer_lock, NULL) != 0){
		printf("Deliverer mutex initialization has failed!\n");
		return -1;
	}
	//initialize all cond variables
	if (pthread_cond_init(&cook_cond, NULL) != 0){
		printf("Producer condition variable initialization has failed!\n");
		return -1;
	}
	if (pthread_cond_init(&oven_cond, NULL) != 0){
		printf("Oven condition variable initialization has failed!\n");
		return -1;
	}
	if (pthread_cond_init(&packer_cond, NULL) != 0){
		printf("Packer condition variable initialization has failed!\n");
		return -1;
	}
	if (pthread_cond_init(&deliverer_cond, NULL) != 0){
		printf("Deliverer condition variable initialization has failed!\n");
		return -1;
	}
	
	
	//create Ncust threads
	pthread_t threads[Ncust];
	printf("Initiating pizzeria for %d customers...\n", Ncust);
	
	//create threads
	for (i = 1; i <= Ncust; i++) {
		//dynamic int pointer to pass order number to threads
		int* dynamic_pointer = malloc(sizeof(int));
		*dynamic_pointer = i;
		//check for potential thread creation errors
		if (pthread_create(&threads[i-1], NULL, pizzeria, (void*)dynamic_pointer) != 0){
			printf("Thread with id: %d, could not be created!\n",i);
			return -1;
		}
	}
	


	//exit main thread only after subthreads are finished
	while(threads_finished != Ncust){
		continue;
	}
	
	//destroy all mutexes
	printf("Destroying mutexes... \n");
	pthread_mutex_destroy(&print_lock);
	pthread_mutex_destroy(&gclock_lock);
	pthread_mutex_destroy(&statistics_lock);
	pthread_mutex_destroy(&cook_lock);
	pthread_mutex_destroy(&oven_lock);
	pthread_mutex_destroy(&packer_lock);
	pthread_mutex_destroy(&deliverer_lock);
	//destroy all cond variables
	printf("Destroying pthread conditions... \n");
	pthread_cond_destroy(&cook_cond);
	pthread_cond_destroy(&oven_cond);
	pthread_cond_destroy(&packer_cond);
	pthread_cond_destroy(&deliverer_cond);
	
	//join threads
	for (j = 1; i <= Ncust; i++) {
	//check for potential thread join errors
		if (pthread_join(threads[j-1], NULL) != 0){
		printf("Thread with id: %d, could not be joined!\n",i);
		return -1;
		}
	}	
	
	sleep(2);
	printf("Pizzeria program has concluded with the following statistics: \n");
	printf("Total income: %d\n", income);
	printf("Successful orders: %d\n", successful_orders);
	printf("Non-successful orders: %d\n", Ncust - successful_orders);
	printf("Amount of plain pizzas: %d\n", plain_pizzas);
	printf("Amount of special pizzas: %d\n", special_pizzas);
	printf("Average order time: %lf\n", total_order_time/Ncust);
	printf("Maximum order time: %lf\n", max_order_time);
	printf("Average cold time: %lf\n", total_cold_time/Ncust);
	printf("Maximum cold time: %lf\n", max_cold_time);
	return 0;
}

void* pizzeria(void* arg) {
	int oid = *(int*)arg;
	int thread_specific_seed = seedp + oid;
	int sp_pizzas;
	int pl_pizzas;
	
	//get order start time
	struct timespec order_start;
	clock_gettime(CLOCK_REALTIME, &order_start);

	
	
	//starting order 
	if (oid != 1) {
		int ordering_time = (rand_r(&thread_specific_seed) % Torderhigh) + Torderlow;
		//add a random amount of ordering time 
		sleep(ordering_time);
	}
	
	//select a random amount of pizzas
	int pizzas = (rand_r(&thread_specific_seed) % Norderhigh) + Norderlow;  
	
	//add a random amount of payment time
	int payment_time = (rand_r(&thread_specific_seed) % Tpaymenthigh) + Tpaymentlow;
	sleep(payment_time);
	
	//random payment variable from 0-100
	int payment_success = (rand_r(&thread_specific_seed) % 100); 
	//decide if payment succedes
	if (payment_success <= Pfail*100) {
		printf("Order with id (%d) has failed.\n", oid);
		pthread_mutex_lock(&statistics_lock);
		threads_finished++;
		pthread_mutex_unlock(&statistics_lock);
		return NULL;
	}
	//obtain sales statistics
	
	for (int i = 0; i < pizzas; i++){
		if (rand_r(&thread_specific_seed) % 100 <= Pplain*100){
			pl_pizzas++;
		}
		else{
			sp_pizzas++;
		}
	}

	
	//order initialization message
	printf("Order (%d) has started for %d pizzas!\n",oid, pizzas);
	
	//repare order      
	//check for an available cook
	pthread_mutex_lock(&cook_lock);
	while (avail_cooks == 0){
		// if there are not enough cooks wait until there are
		pthread_cond_wait(&cook_cond, &cook_lock);
		break;
	}
	//if there is an available cook, then starts preparing order
	avail_cooks--;
	pthread_mutex_unlock(&cook_lock);
	
	//calculate the time needed to prepare all the pizzas
	// each pizza takes time of Tprep to prepare
	int prepTime = pizzas * Tprep;
	sleep(prepTime);
	
	//wait for enough ovens to be available
	pthread_mutex_lock(&oven_lock);
	while (avail_ovens < pizzas){
		pthread_cond_wait(&oven_cond, &oven_lock);
	}
	//put the pizzas in the ovens
	avail_ovens = avail_ovens - pizzas;
	pthread_mutex_unlock(&oven_lock);
	
	//cook gets ready for next order
	pthread_mutex_lock(&cook_lock);
	avail_cooks++;
	// signal in case there is an order waiting to be prepared
	pthread_cond_signal(&cook_cond);
	pthread_mutex_unlock(&cook_lock);
	
	//bake pizzas
	sleep(Tbake);
	
	//check for an available packager
	pthread_mutex_lock(&packer_lock);
	while (avail_packers == 0){
		//if there are not enough packagers wait until there are
		pthread_cond_wait(&packer_cond, &packer_lock);
		break;
	}
	//if there is an available packager, then starts packing order
	avail_packers--;
	pthread_mutex_unlock(&packer_lock);
	
	//calculate the time needed to pack all the pizzas
	//each pizza takes time of Tpack to pack
	int packTime = pizzas * Tpack;
	sleep(packTime);
	
	//free occupied ovens
	pthread_mutex_lock(&oven_lock);
	avail_ovens = avail_ovens + pizzas;
	// signal in case there is an order waiting to be put in the ovens
	pthread_cond_signal(&oven_cond);
	pthread_mutex_unlock(&oven_lock);
	
	//packager gets ready for next order
	pthread_mutex_lock(&packer_lock);
	avail_packers++;
	//cold time starts
	struct timespec cold_time_start;
	clock_gettime(CLOCK_REALTIME, &cold_time_start);
	double packaging_time = (cold_time_start.tv_sec - order_start.tv_sec) + (cold_time_start.tv_nsec - order_start.tv_nsec) / 1000000000.0;
	printf("Order (%d) has been prepared in %lf minutes!\n", oid, packaging_time);

	
	//signal in case there is an order waiting to be packed
	pthread_cond_signal(&packer_cond);
	pthread_mutex_unlock(&packer_lock);
	
	//check for an available deliverer
	pthread_mutex_lock(&deliverer_lock);
	while (avail_deliverers == 0){
		//if there are not enough deliverers wait until there are
		pthread_cond_wait(&deliverer_cond, &deliverer_lock);
		break;
	}
	//if there is an available deliverer, then starts delivery
	avail_deliverers--;
	pthread_mutex_unlock(&deliverer_lock);
	
	//delivery time
	int delTime = (rand_r(&thread_specific_seed) % Tdelhigh) + Tdellow;
	sleep(delTime); //go
	//cold time ends
	struct timespec cold_time_finish;
	clock_gettime(CLOCK_REALTIME, &cold_time_finish);
	sleep(delTime); //return
	
	//deliverer get ready for next order
	pthread_mutex_lock(&deliverer_lock);
	avail_deliverers++;
	//signal in case there is an order waiting to be delivered
	pthread_cond_signal(&deliverer_cond);
	pthread_mutex_unlock(&deliverer_lock);
	
	
	//get order finish time
	struct timespec order_finish;
	clock_gettime(CLOCK_REALTIME, &order_finish);
	
	double order_time = (order_finish.tv_sec - order_start.tv_sec) + (order_finish.tv_nsec - order_start.tv_nsec) / 1000000000.0;
	double cold_time = (cold_time_finish.tv_sec - cold_time_start.tv_sec) + (cold_time_finish.tv_nsec - cold_time_start.tv_nsec) / 1000000000.0;
	
	printf("Order (%d) has been completed in %lf minutes!\n", oid, order_time);
	
	//calculate order statistics 
	pthread_mutex_lock(&statistics_lock);
	special_pizzas += sp_pizzas;
	plain_pizzas += pl_pizzas;
	successful_orders++;
	income += sp_pizzas * Cspecial + pl_pizzas * Cplain;
	
	total_order_time += order_time;
	if (order_time > max_order_time){
		max_order_time = order_time;
	}
	
	total_cold_time += cold_time;
	if (cold_time > max_cold_time){
		max_cold_time = cold_time;
	}
	
	threads_finished++;
	pthread_mutex_unlock(&statistics_lock);
	
	//free space from mallocation
	free(arg);
	return NULL;

}
