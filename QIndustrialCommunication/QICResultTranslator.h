#pragma once

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QException>
#include <functional>
#include "DataFormat.h"
#include "QICResult.h"

class QICResultTranslator
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
	static QICResult<TResult> GetResultFromBytes(const QICResult<QByteArray> &result, std::function<TResult(const QByteArray &)> func)
	{
		try
		{
			if (result.IsSuccess)
			{
				return QICResult<TResult>::CreateSuccessResult(func(result.getContent0()));
			}
			return QICResult<TResult>::CreateFailedResult(result);
		}
		catch (const QException &ex)
		{
			QString errorMessage = QString("Data transform error: Length(%1) %2")
									   .arg(result.getContent0().size())
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
	static QICResult<TResult> GetResultFromArray(const QICResult<QVector<TResult>> &result)
	{
		std::function<TResult(const QVector<TResult>&)> lambda = [](const QVector<TResult>& m) -> TResult { return m.at(0); };
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
	static QICResult<TResult> GetSuccessResultFromOther(const QICResult<TIn> &result, std::function<TResult(TIn)> func)
	{
		if (!result.IsSuccess)
		{
			return QICResult<TResult>::CreateFailedResult(result);
		}
		return QICResult<TResult>::CreateSuccessResult(func(result.getContent0()));
	}

	template <typename TIn>
	static QICResult<> GetResultFromOther(const QICResult<TIn> &result, std::function<QICResult<>(TIn)> func)
	{
		if (!result.IsSuccess)
		{
			return QICResult<>::CreateFailedResult(result);
		}
		return func(result.getContent0());
	}

	template <typename TResult, typename... TArgs>
	static QICResult<TResult> GetResultFromOther(
		const QICResult<TResult> &result,
		std::function<TArgs>... func)
	{
		return InternalGetResultFromOther(result, func...);
	}

private:
	template <typename TResult>
	static QICResult<TResult> InternalGetResultFromOther(
		const QICResult<TResult> &result)
	{
		return result;
	}

	template <typename TResult, typename TIn, typename... TArgs>
	static QICResult<TResult> InternalGetResultFromOther(
		const QICResult<TIn> &result,
		std::function<QICResult<TResult>(TIn)> func,
		std::function<TArgs>... others)
	{
		if (!result.IsSuccess)
		{
			return QICResult<TResult>::CreateFailedResult(result);
		}
		return InternalGetResultFromOther(func(result.getContent0()), others...);
	}
};