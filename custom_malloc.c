#define WSIZE 4 /*Word Size*/
#define DSIZE 8 /*Double Word Size*/
#define CHUNKSIZE (1<<12) /*SIze for expanding the heap*/

#define MAX(x,y) ((x>y)?(x):(y))

/*Free list*/
#define PACK(size,alloc) ((size)|(alloc)) /*Size + allocator bit in HDR or FTR*/

#define GET(p) (*(unsigned int *)(p)) /*Reads and returns the word pointed by p*/
#define PUT(p,val) (*(unsigned int *)(p)=(val)) /*Stores the value in the word into address pointed by p*/

#define GET_SIZE(p) (GET(p) & ~0x7) /*Returns the Size from hdr or ftr*/
#define GET_ALLOC(p) (GET(p) & 0x1) /*Returns the alloc status from hdr or ftr*/
 
/*block pointers - first byte of the payload*/
#define HDRP(bp) ((char *)(bp) - WSIZE) /*blk header*/
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp))- DSIZE) /*blk footer*/

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE))) /*address of next blk*/
#define PREV_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-DSIZE))) /*address of prev blk*/

/*Heap with initial free block*/
void mm_init(void)
{
if((heap_listp = mem_sbrk(4*WSIZE)) == (void*)-1)
	return -1;

	PUT(heap_listp,0);
	PUT(heap_listp + (1*WSIZE),PACK(DSIZE,1));
	PUT(heap_listp + (2*WSIZE),PACK(DSIZE,1));
	PUT(heap_listp + (3*WSIZE),PACK(0,1));
	heap_listp += (2*WSIZE);
	
	if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
		return -1;
		
	return 0;
}

/*Extends heap*/

static void *extend_heap(size_t words)
{
char *bp;
size_t size;

size = (words%2) ? (words+1)*WSIZE : (words)*(WSIZE);
if((long)(bp = mem_sbrk(size)) == -1)
	return NULL;
	
	PUT(HDRP(bp),PACK(size,0));
	PUT(FTRP(bp),PACK(size,0));
	PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1)); /*Epilogue header*/
	
	return coalesce(bp);
}

void mm_free(void *bp)
{
	size_t size = GET_SIZE(HDRP(bp));

	PUT(HDRP(bp),PACK(size,0));
	PUT(FTRP(bp),PACK(size,0));
	coalesce(bp);
}

static void *coalesce(void *bp)
{
	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t size = GET_SIZE(HDRP(bp));
	
	if(prev_alloc && next_alloc)
		return bp;
		
	if(prev_alloc && !next_alloc)
	{
		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
		PUT(HDRP(bp),PACK(size,0));
		PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
	}
	
	if(!prev_alloc && next_alloc)
	{
		size += GET_SIZE(HDRP(PREV_BLKP(bp)));
		PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
		PUT(FTPR(bp),PACK(size,0));
		bp = PREV_BLKP(bp);
	}
	
	if(!prev_alloc && !next_alloc)
	{
		size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
		PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
		PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
		bp = PREV_BLKP(bp);
		
	}
	return bp;
}

static void *find_fit(size_t size)
{
	void *bp;
	
	for(bp=heap_listp;GET_SIZE(HDRP(bp))>0;bp = NEXT_BLKP(bp))
	{
		if(!GET_ALLOC(HDRP(bp)) && (GET_SIZE(HDRP(bp)) > size))
			return bp;
	}
	
	return NULL;
}

void place(void *bp, size_t size)
{
	size_t csize = GET_SIZE(HDRP(bp));

	if(csize - size >= 2* DSIZE)
	{
		PUT(HDRP(bp),PACK(size,1);
		PUT(FTRP(bp),PACK(size,1);
		bp = NEXT_BLKP(bp);
		PUT(HDRP(bp),PACK(csize - size,0));
		PUT(HDRP(bp),PACK(csize - size,0));
	}
	else
	{
		PUT(HDRP(bp),PACK(csize,1);
		PUT(FTRP(bp),PACK(csize,1);
	}
}

void mm_malloc(size_t size)
{
	size_t asize;
	size_t extendsize;
	char *bp;
	
	if(size = 0)
		return NULL;
		
	if(size <= DSIZE)
		asize = 2*DSIZE;
	else
		asize = DSIZE * ((size + DSIZE + (DSIZE - 1))/DSIZE);

	if((bp = find_fit(asize) != NULL)
	{
		place(bp,asize);
		return bp;
	}
	
	extendsize = MAX(asize,CHUNKSIZE);
	if(bp = extend_heap(extendsize) != NULL)
	{
		place(bp,asize);
		return bp;
	}
	else
		return bp;
}