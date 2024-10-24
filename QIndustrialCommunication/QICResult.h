#pragma once

#include <QObject>
#include <QString>
#include <tuple>
#include <type_traits>

/// @brief
/// ���ؽ��
/// IsSuccess �����ǳɹ����ж�
/// Message ���ص���ϸ��Ϣ
/// ErrorCode �������
/// Content ���ص�����
/// @tparam ...T
/// �ɱ�ģ�������variadic template parameter��
/// ...��һ������չ��pack expansion�������������ڱ�ʾһ������ģ�����
template <typename... T>
class QICResult
{
public:
	QICResult() : IsSuccess(false), Message(""), ErrorCode(0) {}
	/// @brief ��עContent
	/// @param isSuccess
	/// @param message
	/// @param errorCode
	/// @param ...content
	QICResult(bool isSuccess, const QString& message, int errorCode, T... content)
		: IsSuccess(isSuccess), Message(message), ErrorCode(errorCode), Content(std::make_tuple(content...))
	{
	}

private:
	/// @brief ����עContent
	/// @param message
	/// @param errorCode
	QICResult(const QString& message, int errorCode)
		: IsSuccess(false), Message(message), ErrorCode(errorCode)
	{
	}

	/// @brief ͨ��������ȡContent�е�Ԫ��
	/// @tparam U Content��Ԫ������ֵ,��0��ʼ
	/// @return
	template <std::size_t Index>
	auto getContent() const
	{
		static_assert(Index < std::tuple_size_v<decltype(Content)>, "Index out of bounds");
		return std::get<Index>(Content);
	}

	/// @brief
	/// ��Contentָ������������ֵ���������
	/// std::is_same_v ���ڱȽ� std::tuple_element_t<Index, std::tuple<T...>>��Ԫ����ָ������λ�õ����ͣ���
	/// std::decay_t<U>������ֵ�����ͣ�ȥ����������ú�cv�޶������Ƿ���ͬ,������ǲ���ͬ��static_assert ��ʧ�ܣ�����һ���������
	/// std::decay_t �������Ƴ����ݹ��������͵Ķ��� const��volatile ���������η�
	/// ��ͨ���ǵ�����Ƚ����������ǲ���ȫ��ͬ������������ͬ������ʱ���õġ�������뱣����Щ���η�����ô����Բ�ʹ�� std::decay_t
	/// @tparam U ��Ӧ��������U�Ķ���
	/// @tparam Index ����
	/// @param value ����ΪU�Ķ���ֵ
	template <std::size_t Index, typename U>
	void setContent(U&& value)
	{
		static_assert(Index < std::tuple_size_v<decltype(Content)>, "Index out of bounds");
		// ����ʱ�������ƥ��
		static_assert(std::is_same_v<std::tuple_element_t<Index, std::tuple<T...>>, std::decay_t<U>>,
			"Type mismatch: The provided type does not match the tuple element type at the specified index.");
		std::get<Index>(Content) = std::forward<U>(value);
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
	/// ...U�Ǻ���ģ��Ĳ�������T����ģ��Ĳ���������������У�T��U�����ǲ�ͬ������
	/// ��ˣ�����Ҫ��ʽ��ָ������Ҫ��������QICResult���ͣ������Ϊʲô����Ҫ��QICResult<T ...>
	/// ����㲻ָ��T�����������޷�ȷ��������ʹ�ú���ģ�����U������ģ�����T���⽫���±������򲻷���Ԥ�ڵ���Ϊ
	/// @tparam ...U
	/// @param other
	/// @return
	template <typename... U>
	static auto CreateFailedResult(const QICResult<U...>& other)
	{
		auto result = QICResult(other.Message, other.ErrorCode);
		return result;
	}

#pragma region Generate getContent setContent convenient method
	// ������������������Ƿ�����ض�����
	template<std::size_t N>
	struct has_index : std::integral_constant<bool, (N < sizeof...(T))> {};

	// getContentN - ֻ��������Чʱ����
	template<std::size_t N>
	auto getContentN() const -> typename std::enable_if_t<has_index<N>::value, std::tuple_element_t<N, std::tuple<T...>>>
	{
		return getContent<N>();
	}
	// setContentN - ֻ��������Чʱ����
	template<std::size_t N, typename U>
	auto setContentN(U&& value) -> typename std::enable_if_t<has_index<N>::value&& std::is_same_v<std::tuple_element_t<N, std::tuple<T...>>, std::decay_t<U>>, void>
	{
		setContent<N>(std::forward<U>(value));
	}

	// ���getContent���� - ʹ��SFINAE(Substitution Failure Is Not An Error)ȷ������ֻ�ڶ�Ӧ��������ʱ����
	template<typename = typename std::enable_if_t<has_index<0>::value>>
	auto getContent0() const { return getContentN<0>(); }
	template<typename = typename std::enable_if_t<has_index<1>::value>>
	auto getContent1() const { return getContentN<1>(); }
	template<typename = typename std::enable_if_t<has_index<2>::value>>
	auto getContent2() const { return getContentN<2>(); }
	template<typename = typename std::enable_if_t<has_index<3>::value>>
	auto getContent3() const { return getContentN<3>(); }
	template<typename = typename std::enable_if_t<has_index<4>::value>>
	auto getContent4() const { return getContentN<4>(); }
	template<typename = typename std::enable_if_t<has_index<5>::value>>
	auto getContent5() const { return getContentN<5>(); }

	// ���setContent���� - ʹ��SFINAE(Substitution Failure Is Not An Error)ȷ������ֻ�ڶ�Ӧ��������ʱ����
	template<typename U, typename = typename std::enable_if_t<has_index<0>::value>>
	void setContent0(U&& value) { setContentN<0>(std::forward<U>(value)); }
	template<typename U, typename = typename std::enable_if_t<has_index<1>::value>>
	void setContent1(U&& value) { setContentN<1>(std::forward<U>(value)); }
	template<typename U, typename = typename std::enable_if_t<has_index<2>::value>>
	void setContent2(U&& value) { setContentN<2>(std::forward<U>(value)); }
	template<typename U, typename = typename std::enable_if_t<has_index<3>::value>>
	void setContent3(U&& value) { setContentN<3>(std::forward<U>(value)); }
	template<typename U, typename = typename std::enable_if_t<has_index<4>::value>>
	void setContent4(U&& value) { setContentN<4>(std::forward<U>(value)); }
	template<typename U, typename = typename std::enable_if_t<has_index<5>::value>>
	void setContent5(U&& value) { setContentN<5>(std::forward<U>(value)); }
#pragma endregion

private:
	std::tuple<T...> Content;

public:
	bool IsSuccess;
	QString Message;
	int ErrorCode;
};

