
#include "voltageComparator.h"


// nRF5x
#include <drivers/comparator.h>

namespace {

VCompareResult convertComparisonResult(bool result) {
	if (result) return VCompareResult::AboveRef;
	else        return VCompareResult::BelowRef;
}
}

VCompareResult VoltageComparator::compareToLowThreshold() {
	bool intermediateResult = Comparator::initCompareAndShutdown(ComparatorReferenceVolts::V1_2);
	return convertComparisonResult(intermediateResult);
}

VCompareResult VoltageComparator::compareToHighThreshold() {
	bool intermediateResult = Comparator::initCompareAndShutdown(ComparatorReferenceVolts::V2_4);
	return convertComparisonResult(intermediateResult);
}
