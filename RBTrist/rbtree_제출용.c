#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h> //for kmalloc
#include <linux/time.h>
#include <linux/rbtree.h>
#include <linux/list.h>
#include <linux/random.h>  // get_random_int()
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define FALSE 0
#define TRUE 1

#define THREAD_NUM 8
#define TREE_NUM 100000
#define LIST_NUM 100000
#define MAX_BUFFER 100000   
#define BILLION 1000000000

unsigned long long insert_time = 0;
unsigned long long search_time = 0;
unsigned long long delete_time = 0;

spinlock_t insert_lock;

int fin_flag = 0;

struct thread_par {
	struct rb_root *my_root;
	int idx;
};


struct my_tree {
	struct rb_node node;
	int key;
};

int init_rb_data[TREE_NUM];			 // 초기 rb_tree 에 insert 할 data 를 담을 arr
int rb_list_data[LIST_NUM];			 // rb list 에 insert 할 data 를 담을 arr
int init_rb_data_idx = 0;  
int rb_data_idx = 0;  


unsigned long long calculate(struct timespec64 *tick, struct timespec64 *tock)
{
	unsigned long long total_time = 0, time_delay = 0; 
	long temp;
	long temp_n;

	if(tock->tv_nsec >= tick->tv_nsec) {

		temp = tock->tv_sec - tick->tv_sec;
		temp_n = tock->tv_nsec - tick->tv_nsec;
	}
	else {
		temp = tock->tv_sec - tick->tv_sec -1;
		temp_n = BILLION + tock->tv_nsec - tick->tv_nsec;

	}

	time_delay = BILLION * temp + temp_n;
	__sync_fetch_and_add(&total_time, time_delay);

	return total_time;
}


/* normal rbtree insert, search, delete */
int rb_insert(struct rb_root *root, struct my_tree *data) 
{
	struct rb_node **new = &(root->rb_node), *parent = NULL;
	/* Figure out "where" to put new node */
	while(*new) {
		struct my_tree *this = container_of(*new, struct my_tree, node);
		parent = *new;

		if(this->key > data->key)
			new = &((*new)->rb_left);
		else if(this->key < data->key)
			new = &((*new)->rb_right);
		else
			return FALSE;
	}

	rb_link_node(&data->node, parent, new); /*relinking*/
	rb_insert_color(&data->node, root);     /*recoloring & rebalancing*/
	
	return TRUE;
}


struct my_tree *rb_search(struct rb_root *root, int key) 
{
	struct rb_node *node = root->rb_node;

	while(node) {
		struct my_tree *data = container_of(node, struct my_tree, node);

		if(data->key > key)
			node = node->rb_left;
		else if(data->key < key)
			node = node->rb_right;
		else
			return data;
	}
	return NULL;
}


void rb_delete(struct rb_root *root, int key)
{
	struct my_tree *data = rb_search(root, key);

	if(data) {
		rb_erase(&data->node, root);
		kfree(data);
	}
}


int thread_func(void *par)
{
	struct thread_par *data = (struct thread_par *)par;
	int data_num = LIST_NUM / THREAD_NUM;
	int start = data->idx * data_num;
	int end = (data->idx + 1) * data_num;
	int i;
	struct my_tree *new;

	for(i=start; i<end; i++){
		new = kmalloc(sizeof(struct my_tree), GFP_KERNEL);
		if(!new)
			return -1;

		new->key = rb_list_data[i];
	
		/* rbroot 에 하나의 thread 만 접근할 수 있으므로 rb_insert 부르기 전에 lock 을 걸어준다 */
		spin_lock(&insert_lock);
		rb_insert(data->my_root, new);  // ret is true or false
		spin_unlock(&insert_lock);
	}
	__sync_fetch_and_add(&fin_flag, 1);
	return 0;
}


void struct_example(void) 
{
	spin_lock_init(&insert_lock);
	struct rb_root my_root = RB_ROOT;
	struct my_tree *find_node = NULL;  // search node

	int i, ret;

	struct timespec64 tick, tock;

	int num = 0;

	// normal rbtree insert
	printk("\nrb tree Insert");
	struct my_tree *new;

	struct thread_par parms[THREAD_NUM];

	// 초기 rbtree 100,000 개
	for (i=0; i<TREE_NUM+LIST_NUM; i++) {
		new = kmalloc(sizeof(struct my_tree), GFP_KERNEL);
		if(!new)
			return;

		if (i % 2 != 0) {  
			new->key = num;

			init_rb_data[init_rb_data_idx++] = num;
			ret = rb_insert(&my_root, new);  // ret is true or false
		}	
		else {
			rb_list_data[rb_data_idx++] = num;  // 나중에 insert 할 data
		}

		num += get_random_int() % 10 + 1;	
	}

	// customized rb list insert
	/* rb list insert 100,000 개 */
	printk("\nrb tree Insert");

	ktime_get_ts64(&tick); 
	for(i=0; i<THREAD_NUM; i++) { 
		parms[i].my_root = &my_root;
		parms[i].idx = i;
		kthread_run(thread_func, (void*)&parms[i], "insert_thread");
	}
	while(__sync_fetch_and_add(&fin_flag, 0) != THREAD_NUM){
	}
	ktime_get_ts64(&tock); 
	insert_time = calculate(&tick, &tock);
	

	///////////////////////////////////////////////////////////////////


	/* rb tree search
	 * 초기 rbtree data 5000개 + 나중에 넣은 rbtree data 5000개 
	 */
	printk("\nrb mixed tree Search");
	ktime_get_ts64(&tick); 

	int target_idx;
	for(i=0; i<TREE_NUM/2; i++) {  
		target_idx = get_random_int() % TREE_NUM;	

		find_node = rb_search(&my_root, init_rb_data[target_idx]);
		if(!find_node)
			return;
	}

	for(i=0; i<LIST_NUM/2; i++) {  
		target_idx = get_random_int() % LIST_NUM;	

		find_node = rb_search(&my_root, rb_list_data[target_idx]);
		if(!find_node)
			return;
	}

	ktime_get_ts64(&tock); 
	search_time = calculate(&tick, &tock);


	///////////////////////////////////////////////////////////////////


	/*rb_tree delete node*/
	ktime_get_ts64(&tick); 

	for(i=0; i<TREE_NUM; i++) {  // 100000
		rb_delete(&my_root, init_rb_data[i]);
	}

	for(i=0; i<LIST_NUM; i++) {  // 100000 
		rb_delete(&my_root, rb_list_data[i]);
	}
	ktime_get_ts64(&tock); 
	delete_time = calculate(&tick, &tock);

}


static int __init hello_module_init(void) 
{
	struct_example();
	printk("module init\n");
	return 0;

}

static void __exit hello_module_cleanup(void) 
{
	printk("rb insert time: %llu ns\n", insert_time);
	printk("rb search time: %llu ns\n", search_time);
	printk("rb delete time: %llu ns\n\n", delete_time);

	printk("Bye Module\n");
}

module_init(hello_module_init);
module_exit(hello_module_cleanup);
MODULE_LICENSE("GPL");

