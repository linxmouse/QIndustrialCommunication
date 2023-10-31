#pragma once

#include <QObject>
#include <QString>
#include <tuple>

/// @brief 
/// 返回结果
/// IsSuccess 具有是成功的判断
/// Message 返回的详细消息
/// ErrorCode 错误代码
/// Content 返回的内容
/// @tparam ...T 
/// 可变模板参数（variadic template parameter）
/// ...是一个包扩展（pack expansion）操作符，用于表示一个或多个模板参数
template <typename... T>
class QICResult
{
public:
	QICResult() : IsSuccess(false), Message(""), ErrorCode(0) {}
	/// @brief 关注Content
	/// @param isSuccess 
	/// @param message 
	/// @param errorCode 
	/// @param ...content 
	QICResult(bool isSuccess, const QString& message, int errorCode, T... content)
		: IsSuccess(isSuccess), Message(message), ErrorCode(errorCode), Content(std::make_tuple(content...))
	{
	}
private:
	/// @brief 不关注Content
	/// @param message 
	/// @param errorCode 
	QICResult(const QString& message, int errorCode)
		: IsSuccess(false), Message(message), ErrorCode(errorCode)
	{
	}

public:
	QString ToMessageShowString() const
	{
		return QString("IsSuccess: %1, Message: %2, ErrorCode: %3")
			.arg(IsSuccess)
			.arg(Message)
			.arg(ErrorCode);
	}

	template <typename... U>
	void CopyErrorFromOther(const QICResult<U...>& other)
	{
		this->IsSuccess = other.IsSuccess;
		this->Message = other.Message;
		this->ErrorCode = other.ErrorCode;
	}

	static QICResult CreateSuccessResult(T... content)
	{
		return QICResult(true, "Success", 0, content...);
	}

	static QICResult CreateFailedResult(const QString& message)
	{
		auto result = QICResult(message, -1);
		return result;
	}

	/// @brief 
	/// ...U是函数模板的参数，而T是类模板的参数。在这个函数中，T和U可能是不同的类型
	/// 因此，你需要显式地指定你想要构造哪种QICResult类型，这就是为什么你需要用QICResult<T ...>
	/// 如果你不指定T，编译器将无法确定你是想使用函数模板参数U还是类模板参数T，这将导致编译错误或不符合预期的行为
	/// @tparam ...U 
	/// @param other 
	/// @return 
	template <typename ... U>
	static auto CreateFailedResult(const QICResult<U ...>& other)
	{
		auto result = QICResult(other.Message, other.ErrorCode);
		return result;
	}

	/// @brief 通过索引获取Content中的元素
	/// @tparam U Content的元组索引值,从0开始
	/// @return 
	template <std::size_t Index>
	auto getContent() const
	{
		static_assert(Index<std::tuple_size_v<decltype(Content)>, "Index out of bounds");
		return std::get<Index>(Content);
	}

	/// @brief 
	/// 对Content指定的索引设置值，检查类型
	/// std::is_same_v 用于比较 std::tuple_element_t<Index, std::tuple<T...>>（元组在指定索引位置的类型）与 
	/// std::decay_t<U>（传入值的类型，去掉顶层的引用和cv限定符）是否相同,如果它们不相同，static_assert 会失败，产生一个编译错误
	/// std::decay_t 是用来移除传递过来的类型的顶层 const、volatile 和引用修饰符
	/// 这通常是当你想比较两个可能是不完全相同但基础类型相同的类型时有用的。如果你想保持这些修饰符，那么你可以不使用 std::decay_t
	/// @tparam U 对应索引类型U的对象
	/// @tparam Index 索引
	/// @param value 类型为U的对象值
	template <std::size_t Index, typename U>
	void setContent(U&& value)
	{
		// 编译时检查类型匹配
		static_assert(std::is_same_v<std::tuple_element_t<Index, std::tuple<T...>>, std::decay_t<U>>,
			"Type mismatch: The provided type does not match the tuple element type at the specified index.");
		std::get<Index>(Content) = std::forward<U>(value);
	}

public:
	bool IsSuccess;
	QString Message;
	int ErrorCode;
	std::tuple<T...> Content;
};
