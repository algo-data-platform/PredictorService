
struct Referer {
	std::string ip;
	int port;
};

/*
 * 负载均衡策略
 *
 */
class LoadBalanceIf {
	// 当列表更新的时候的处理方法	
	virtual void onRefresh(std::vector<Referer> referers) = 0;

	// 选择一个节点	
	virtual Referer select(Request request) = 0;

	// 返回一个可以选节点列表 
	virtual void selectToHolder(Request request, std::vector<Referer>& result) = 0;
};
