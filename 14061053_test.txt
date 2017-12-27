const int zero = 0, one = 1;
const char ch = 'i';
int nums[20];
int pn;
int func(int a,char b){
	const char con = '_';
	pn = pn+1;
	nums[pn] = a;
	nums[pn-1+zero] = nums[pn];
	printf(b);
	printf(con);
	return (a);
}
void test(int a){
	if (a>zero) {
		a=a-2;
		printf("a=",a+one);
		test((a+one)*1);
	} else if (a>=0) {
		printf("a=",a);
		return;
	} else return;
}
char retchr(char a,int k){
	if (k==zero) {
		return (a);
	}
	if (k == one )
		return (a+1);
	return ('t');
}
void main(){
	const int k = +2, mo = -1;
	int a,i;
	char w;
	a = 4;
	test(a);
	for(i=0;i<=a;i=i+1){
		printf(i);
		printf(ch);
	}
	i=3;
	do{
		test(a/one);
		i=i-1;
		i=func(i,'f');
    } while (i > 0)
	for(i=0;i<=pn;i=i+1)
	printf(nums[i]);
	printf(mo+k*(mo+k));
	w=retchr(ch,0);
	printf(w);
	w=retchr(w,1);
	printf(w);
	w=retchr(ch,2);
	printf(w);
}
