namespace abc {

	template <typename Future, typename Function, typename... Args>
	class future : public std::future<T> {
	public:
		std::future<std::invoke_result_t<std::decay_t<Function>, std::decay_t<Args>...>>
		then(Function&& func, Args&&... args) {
			return std::async([] (fut, std::move(f), std::move(args)) {
				fut.wait();
				return f(args...);
			});
		}
	};
	
}

