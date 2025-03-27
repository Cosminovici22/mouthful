#!/usr/bin/sh

if [ -z ${CLEANUP+x} ]; then
	CLEANUP=1
fi

if [ -z ${DEBUG+x} ]; then
	DEBUG=1
fi

TESTS_DIR=$(dirname "$0")
BUILD_DIR="${TESTS_DIR}/../build"

MFGEN="${BUILD_DIR}/mfgen"
MFTOK="${BUILD_DIR}/mftok"

passed_tests=0
all_tests=0

echo "RUNNING TESTS"

for test in "${TESTS_DIR}"/test[0-9]*; do
	tokens="${test}/tokens"
	lexer="${test}/lexer"

	echo
	echo "GENERATING $lexer"
	"$MFGEN" "$tokens" "$lexer"
	if [ $? -ne 0 ]; then
		continue
	fi

	for sample in "${test}"/sample[0-9]*; do
		output="$(echo "$sample" | sed "s/sample/output/")"
		reference="$(echo "$sample" | sed "s/sample/reference/")"

		printf '%s' "TESTING $lexer AGAINST $sample: "
		"$MFTOK" "$lexer" "$sample" > "$output" 2>&1

		temp="$(diff "$output" "$reference")"
		if [ $? -ne 0 ]; then
			echo "FAILED"
			if [ $DEBUG -ne 0 ]; then
				echo "$output VERSUS $reference :"
				echo "$temp"
			fi
		else
			passed_tests=$((passed_tests + 1))
			echo "PASSED"
		fi

		if [ $CLEANUP -ne 0 ]; then
			rm -f "$output"
		fi

		all_tests=$((all_tests + 1))
	done

	if [ $CLEANUP -ne 0 ]; then
		rm -f "$lexer"
	fi
done

echo
echo "PASSED TESTS: $passed_tests/$all_tests"
