
#include "voltageComparator.h"


// nRF5x
#include <drivers/comparator.h>

namespace {

VCompareResult convertComparisonResult(bool isAbove) {
	if (isAbove) return VCompareResult::AboveRef;
	else        return VCompareResult::BelowRef;
}
}

VCompareResult VoltageComparator::compareToLowThreshold() {
	bool isAbove = Comparator::initCompareAndShutdown(ComparatorReferenceVolts::V1_2);
	return convertComparisonResult(isAbove);
}

VCompareResult VoltageComparator::compareToHighThreshold() {
	bool isAbove = Comparator::initCompareAndShutdown(ComparatorReferenceVolts::V2_4);
	return convertComparisonResult(isAbove);
}
