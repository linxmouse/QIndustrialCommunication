#include <QObject>
#include <QSysInfo>

class BytesOrderHelper
{
public:
	static bool isLittleEndian() {
		return QSysInfo::ByteOrder == QSysInfo::LittleEndian;
	}

	static bool isBigEndian() {
		return QSysInfo::ByteOrder == QSysInfo::BigEndian;
	}

	template<typename T>
	static T toNetworkOrder(T value) {
		if (isLittleEndian()) return qToBigEndian(value);
		// ����Ѿ��Ǵ�ˣ�����ת��
		return value;
	}

	template<typename T>
	static T fromNetworkOrder(T value) {
		if (isLittleEndian()) return qFromBigEndian(value);
		// ����Ѿ��Ǵ�ˣ�����ת��
		return value;
	}
};
