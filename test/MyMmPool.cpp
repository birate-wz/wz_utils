
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#define  MP_ALIGNMENT               32
#define  MP_PAGE_SIZE               4096
#define  MP_MAX_ALLOC_FROM_POOL     (MP_PAGE_SIZE - 1)
#define  mp_align(n, alignment)     (((n)+(alignment-1)) & ~(alignment - 1))
#define  mp_align_ptr(p, alignment)   (void*)((((size_t)p) + (alignment-1)) & ~(alignment-1))


typedef struct mp_large_s
{
    struct mp_large_s *next;
    void *data;
}mp_large_s;

typedef struct mp_node_s
{
    unsigned char *last;      // 指向使用的内存的最后
    unsigned char *end;       // 指向小内存的末尾
    struct mp_node_s *next;   //指向下一个小的内存池
    size_t failed;            //失败的次数
}mp_node_s;

typedef struct mp_pool_s {
    size_t max;           //最大的size  4096
    mp_node_s *small;
    mp_large_s *large;

    mp_node_s head[0];
}mp_pool_s;

class MyMmPool
{
public:
    MyMmPool(size_t size);
    void* mp_alloc(size_t size);
    ~MyMmPool();

private:
    void *mp_alloc_large(size_t size);
    void *mp_alloc_block(size_t size);
    void mp_reset_pool();
    void mp_free();

    mp_pool_s *mypool;
    /* data */
};

MyMmPool::MyMmPool(size_t size):mypool(nullptr)
{
    int ret = posix_memalign((void **)&mypool, MP_ALIGNMENT, size + sizeof(mp_pool_s) + sizeof(mp_node_s));
    if (ret) {
        printf("error: ret = %d", ret);
    }

    mypool->max = size < MP_MAX_ALLOC_FROM_POOL ? size : MP_MAX_ALLOC_FROM_POOL;
    mypool->small = mypool->head;
    mypool->large = NULL;
    mypool->head->last = (unsigned char*)mypool + sizeof(mp_pool_s) + sizeof(mp_node_s);
    mypool->head->end = mypool->head->last + size;
    mypool->head->failed = 0;
}

void* MyMmPool::mp_alloc_block(size_t size) {
    printf("allocate small block!\n");
    unsigned char *m;
    mp_node_s *h = mypool->head;
    size_t psize = (size_t)(h->end - (unsigned char *)h);

    int ret = posix_memalign((void**)&m, MP_ALIGNMENT, psize);
    if (ret) return nullptr;

    mp_node_s *p, *new_node, *current;
    new_node = (mp_node_s*)m;
    new_node->end = m + psize;
    new_node->next = NULL;
    new_node->failed = 0;

    m += sizeof(mp_node_s);
    m = (unsigned char *)mp_align_ptr(m, MP_ALIGNMENT);
    new_node->last = m + size;

    current = mypool->small;
    for (p = current; p->next; p = p->next) {
        if (p->failed++ > 4 )
        current = p->next;
    } 
    p->next = new_node;

    mypool->small = current ? current:new_node;
    return m;
}

void* MyMmPool::mp_alloc_large(size_t size) {
    printf("allocate large block!\n");
    void *p = malloc(size);
    if (p == nullptr) return nullptr;
    size_t n = 0;
    mp_large_s *large;
    for (large = mypool->large; large; large=large->next) {
        if (large->data == nullptr) {
            large->data = p;
            return p;
        }
        if (n++ > 3) break;
    }

    large = (mp_large_s *)mp_alloc(sizeof(mp_large_s));
    if (large == nullptr) {
        free(p);
        return nullptr;
    }
    large->data = p;
    large->next = mypool->large;
    mypool->large = large;
    return p;
}

void* MyMmPool::mp_alloc(size_t size) {
    printf("allocate block!\n");
    unsigned char *m;
    mp_node_s *p;

    if (size <= mypool->max) {
        p = mypool->small;
        do {
            m = (unsigned char *)mp_align_ptr(p->last, MP_ALIGNMENT);
            if ((size_t)(p->end - m) >= size) {
                p->last =  m + size;
                return m;  
            }
            p = p->next;
        }while(p);

        return mp_alloc_block(size);
    }
    return mp_alloc_large(size);
}

void MyMmPool::mp_free() {
    printf("free block!\n");
    mp_node_s *h, *n;
    mp_large_s *large;

    for (large = mypool->large; large; large = large->next) {
        if (large->data) {
            free(large->data);
            large->data = nullptr;
        }
    }

    h = mypool->head->next;   // 头节点的下一个为开始位置
    while(h) {
        n = h->next;
        free(h);
        h = n;
    }
    free(mypool);
}

void MyMmPool::mp_reset_pool() {
	struct mp_node_s *h;
	struct mp_large_s *l;

	for (l = mypool->large; l; l = l->next) {
		if (l->data) {
			free(l->data);
            l->data = nullptr;
		}
	}
	mypool->large = NULL;

	for (h = mypool->head; h; h = h->next) {
		h->last = (unsigned char *)h + sizeof(mp_node_s);
	}
}

MyMmPool::~MyMmPool()
{
    mp_free();
}


int main() {
    size_t size = 1 << 12;
    MyMmPool pool(size);

    for (int i = 0; i<10; i++) {
        void *p = pool.mp_alloc(512);
        if (p==nullptr) printf("failed to allcate");
    }

    return 0;
}