#pragma once

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QException>
#include <functional>
#include "DataFormat.h"
#include "QICResult.h"

class ByteConverterHelper
{
public:
	/**
	 * @brief Converts the result from a QByteArray to TResult using a given function.
	 *
	 * @tparam TResult Type of the return value.
	 * @param result QICResult object containing a QByteArray.
	 * @param func Function to convert from QByteArray to TResult.
	 * @return A QICResult object containing TResult.
	 * @throw QException If the conversion function throws an exception.
	 */
	template <typename TResult>
	static QICResult<TResult> GetResultFromBytes(const QICResult<QByteArray>& result, std::function<TResult(const QByteArray&)> func)
	{
		try
		{
			if (result.IsSuccess)
			{
				return QICResult<TResult>::CreateSuccessResult(func(result.GetContent1()));
			}
			return QICResult<TResult>::CreateFailedResult(result);
		}
		catch (const QException& ex)
		{
			QString errorMessage = QString("Data transform error: Length(%1) %2")
				.arg(result.GetContent1().size())
				.arg(ex.what());
			return QICResult<TResult>::CreateFailedResult(errorMessage);
		}
	}

	/**
	 * @brief Extracts the first element from a QVector inside a QICResult.
	 *
	 * @tparam TResult Type of the return value.
	 * @param result QICResult object containing a QVector of TResult.
	 * @return A QICResult object containing the first element of type TResult.
	 */
	template <typename TResult>
	static QICResult<TResult> GetResultFromArray(const QICResult<QVector<TResult>>& result)
	{
		auto lambda = [](const QVector<TResult>& m) -> TResult { return m[0]; };
		return GetSuccessResultFromOther<TResult, QVector<TResult>>(result, lambda);
	}

	/**
	 * @brief Converts the result from TIn to TResult using a given function.
	 *
	 * This function is useful for converting from one QICResult type to another.
	 *
	 * @tparam TResult Type of the return value.
	 * @tparam TIn Type of the input value.
	 * @param result QICResult object containing TIn.
	 * @param func Function to convert from TIn to TResult.
	 * @return A QICResult object containing TResult.
	 */
	template <typename TResult, typename TIn>
	static QICResult<TResult> GetSuccessResultFromOther(const QICResult<TIn>& result, std::function<TResult(TIn)> func)
	{
		if (!result.IsSuccess)
		{
			return QICResult<TResult>::CreateFailedResult(result);
		}
		return QICResult<TResult>::CreateSuccessResult(func(result.GetContent1()));
	}

#if 0 // 暂时未使用
	template <typename TIn>
	static QICResult<> GetResultFromOther(const QICResult<TIn>& result, std::function<QICResult<>(TIn)> func)
	{
		if (!result.IsSuccess)
		{
			return QICResult<>::CreateFailedResult(result);
		}
		return func(result.GetContent1());
	}

	/*template <typename TResult, typename TIn>
	static QICResult<TResult> GetResultFromOther(const QICResult<TIn>& result, std::function<QICResult<TResult>(TIn)> func)
	{
		if (!result.IsSuccess)
		{
			return QICResult<TResult>::CreateFailedResult(result);
		}
		return func(result.GetContent1());
	}

	template <typename TResult, typename TIn1, typename TIn2>
	static QICResult<TResult> GetResultFromOther(const QICResult<TIn1>& result,
		std::function<QICResult<TIn2>(TIn1)> trans1,
		std::function<QICResult<TResult>(TIn2)> trans2)
	{
		if (!result.IsSuccess)
		{
			return QICResult<TResult>::CreateFailedResult(result);
		}
		QICResult<TIn2> intermediateResult = trans1(result.GetContent1());
		if (!intermediateResult.IsSuccess)
		{
			return QICResult<TResult>::CreateFailedResult(intermediateResult);
		}

		return trans2(intermediateResult.GetContent1());
	}

	template <typename TResult, typename TIn1, typename TIn2, typename TIn3>
	static QICResult<TResult> GetResultFromOther(
		const QICResult<TIn1>& result,
		std::function<QICResult<TIn2>(TIn1)> trans1,
		std::function<QICResult<TIn3>(TIn2)> trans2,
		std::function<QICResult<TResult>(TIn3)> trans3)
	{
		if (!result.IsSuccess)
		{
			return QICResult<TResult>::CreateFailedResult(result);
		}
		QICResult<TIn2> intermediateResult1 = trans1(result.GetContent1());
		if (!intermediateResult1.IsSuccess)
		{
			return QICResult<TResult>::CreateFailedResult(intermediateResult1);
		}
		QICResult<TIn3> intermediateResult2 = trans2(intermediateResult1.GetContent1());
		if (!intermediateResult2.IsSuccess)
		{
			return QICResult<TResult>::CreateFailedResult(intermediateResult2);
		}

		return trans3(intermediateResult2.GetContent1());
	}

	template <typename TResult, typename TIn1, typename TIn2, typename TIn3, typename TIn4>
	static QICResult<TResult> GetResultFromOther(
		const QICResult<TIn1>& result,
		std::function<QICResult<TIn2>(TIn1)> trans1,
		std::function<QICResult<TIn3>(TIn2)> trans2,
		std::function<QICResult<TIn4>(TIn3)> trans3,
		std::function<QICResult<TResult>(TIn4)> trans4)
	{
		if (!result.IsSuccess)
		{
			return QICResult<TResult>::CreateFailedResult(result);
		}
		QICResult<TIn2> intermediateResult1 = trans1(result.GetContent1());
		if (!intermediateResult1.IsSuccess)
		{
			return QICResult<TResult>::CreateFailedResult(intermediateResult1);
		}
		QICResult<TIn3> intermediateResult2 = trans2(intermediateResult1.GetContent1());
		if (!intermediateResult2.IsSuccess)
		{
			return QICResult<TResult>::CreateFailedResult(intermediateResult2);
		}
		QICResult<TIn4> intermediateResult3 = trans3(intermediateResult2.GetContent1());
		if (!intermediateResult3.IsSuccess)
		{
			return QICResult<TResult>::CreateFailedResult(intermediateResult3);
		}

		return trans4(intermediateResult3.GetContent1());
	}

	template <typename TResult, typename TIn1, typename TIn2, typename TIn3, typename TIn4, typename TIn5>
	QICResult<TResult> GetResultFromOther(
		const QICResult<TIn1>& result,
		std::function<QICResult<TIn2>(TIn1)> trans1,
		std::function<QICResult<TIn3>(TIn2)> trans2,
		std::function<QICResult<TIn4>(TIn3)> trans3,
		std::function<QICResult<TIn5>(TIn4)> trans4,
		std::function<QICResult<TResult>(TIn5)> trans5)
	{
		if (!result.IsSuccess)
		{
			return QICResult<TResult>::CreateFailedResult(result);
		}
		QICResult<TIn2> intermediateResult1 = trans1(result.GetContent1());
		if (!intermediateResult1.IsSuccess)
		{
			return QICResult<TResult>::CreateFailedResult(intermediateResult1);
		}
		QICResult<TIn3> intermediateResult2 = trans2(intermediateResult1.GetContent1());
		if (!intermediateResult2.IsSuccess)
		{
			return QICResult<TResult>::CreateFailedResult(intermediateResult2);
		}
		QICResult<TIn4> intermediateResult3 = trans3(intermediateResult2.GetContent1());
		if (!intermediateResult3.IsSuccess)
		{
			return QICResult<TResult>::CreateFailedResult(intermediateResult3);
		}
		QICResult<TIn5> intermediateResult4 = trans4(intermediateResult3.GetContent1());
		if (!intermediateResult4.IsSuccess)
		{
			return QICResult<TResult>::CreateFailedResult(intermediateResult4);
		}

		return trans5(intermediateResult4.GetContent1());
	}*/

	template <typename TResult, typename... TArgs>
	static QICResult<TResult> GetResultFromOther(
		const QICResult<TResult>& result,
		std::function<TArgs>... func)
	{
		return InternalGetResultFromOther(result, func...);
	}

private:
	template <typename TResult>
	static QICResult<TResult> InternalGetResultFromOther(
		const QICResult<TResult>& result)
	{
		return result;
	}

	template <typename TResult, typename TIn, typename... TArgs>
	static QICResult<TResult> InternalGetResultFromOther(
		const QICResult<TIn>& result,
		std::function<QICResult<TResult>(TIn)> func,
		std::function<TArgs>... others)
	{
		if (!result.IsSuccess)
		{
			return QICResult<TResult>::CreateFailedResult(result);
		}
		return InternalGetResultFromOther(func(result.GetContent1()), others...);
	}
#endif // 0 // 暂时未使用

};