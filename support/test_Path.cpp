/*
 * Test suite for the Path class
 * Particularly focuses on path normalization behavior
 */

#include "Path.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace Storage;

// Test counter and reporting
static int g_testCount = 0;
static int g_failureCount = 0;

#define TEST_ASSERT(condition, message) \
	do { \
		g_testCount++; \
		if (!(condition)) { \
			printf("FAIL [%d]: %s\n", g_testCount, message); \
			g_failureCount++; \
		} else { \
			printf("PASS [%d]\n", g_testCount); \
		} \
	} while (0)

// ============================================================================
// Basic Constructor and Initialization Tests
// ============================================================================

void test_default_constructor()
{
	Path path;
	TEST_ASSERT(path.InitCheck() != 0, "Default constructor should create uninitialized path");
	TEST_ASSERT(path.String() == NULL, "Uninitialized path should have NULL string");
}

void test_constructor_with_absolute_path()
{
	Path path("/tmp");
	TEST_ASSERT(path.InitCheck() == 0, "Absolute path constructor should initialize successfully");
	TEST_ASSERT(strcmp(path.String(), "/tmp") == 0, "Path string should match input");
}

void test_constructor_with_leaf()
{
	Path path("/tmp", "testfile.txt");
	TEST_ASSERT(path.InitCheck() == 0, "Constructor with leaf should initialize successfully");
	TEST_ASSERT(strcmp(path.String(), "/tmp/testfile.txt") == 0, "Path should combine directory and leaf");
}

void test_copy_constructor()
{
	Path original("/home/user");
	Path copy(original);
	TEST_ASSERT(copy.InitCheck() == 0, "Copy constructor should copy initialized path");
	TEST_ASSERT(strcmp(copy.String(), original.String()) == 0, "Copied path should equal original");
}

// ============================================================================
// Normalization Tests - Trailing Slashes
// ============================================================================

void test_normalize_trailing_slash()
{
	Path path("/tmp/", NULL, true);
	TEST_ASSERT(path.InitCheck() == 0, "Normalization of trailing slash should succeed");
	TEST_ASSERT(strcmp(path.String(), "/tmp") == 0, "Trailing slash should be removed by normalization");
}

void test_root_path_no_normalization_needed()
{
	Path path("/", NULL, true);
	TEST_ASSERT(path.InitCheck() == 0, "Root path should normalize successfully");
	TEST_ASSERT(strcmp(path.String(), "/") == 0, "Root path should remain /");
}

// ============================================================================
// Normalization Tests - Double Slashes
// ============================================================================

void test_normalize_double_slash()
{
	Path path("//tmp", NULL, true);
	TEST_ASSERT(path.InitCheck() == 0, "Double slash normalization should succeed");
	TEST_ASSERT(strcmp(path.String(), "/tmp") == 0, "Double slash should become single slash");
}

void test_normalize_multiple_double_slashes()
{
	Path path("/tmp//test//file", NULL, true);
	TEST_ASSERT(path.InitCheck() == 0, "Multiple double slashes should normalize");
	TEST_ASSERT(strcmp(path.String(), "/tmp/test/file") == 0, "All double slashes should be collapsed");
}

void test_normalize_consecutive_slashes()
{
	Path path("/tmp///test////file", NULL, true);
	TEST_ASSERT(path.InitCheck() == 0, "Consecutive slashes should normalize");
	TEST_ASSERT(strcmp(path.String(), "/tmp/test/file") == 0, "Consecutive slashes should be collapsed");
}

// ============================================================================
// Normalization Tests - Dot and Double Dot
// ============================================================================

void test_normalize_single_dot()
{
	Path path("/tmp/./file", NULL, true);
	TEST_ASSERT(path.InitCheck() == 0, "Single dot normalization should succeed");
	TEST_ASSERT(strcmp(path.String(), "/tmp/file") == 0, "/./ should be removed");
}

void test_normalize_double_dot()
{
	Path path("/tmp/test/../file", NULL, true);
	TEST_ASSERT(path.InitCheck() == 0, "Double dot normalization should succeed");
	TEST_ASSERT(strcmp(path.String(), "/tmp/file") == 0, "/../ should resolve to parent directory");
}

void test_normalize_multiple_dots()
{
	Path path("/a/b/c/../../file", NULL, true);
	TEST_ASSERT(path.InitCheck() == 0, "Multiple double dots should normalize");
	TEST_ASSERT(strcmp(path.String(), "/a/file") == 0, "Multiple /../ should resolve correctly");
}

void test_normalize_dot_at_end()
{
	Path path("/tmp/.", NULL, true);
	TEST_ASSERT(path.InitCheck() == 0, "Trailing dot normalization should succeed");
	TEST_ASSERT(strcmp(path.String(), "/tmp") == 0, "Trailing /. should be removed");
}

void test_normalize_double_dot_at_end()
{
	Path path("/tmp/test/..", NULL, true);
	TEST_ASSERT(path.InitCheck() == 0, "Trailing double dot normalization should succeed");
	TEST_ASSERT(strcmp(path.String(), "/tmp") == 0, "Trailing /.. should resolve to parent");
}

// ============================================================================
// Normalization Tests - Complex Cases
// ============================================================================

void test_normalize_complex_path()
{
	Path path("/home//user/./documents/../downloads/file.txt", NULL, true);
	TEST_ASSERT(path.InitCheck() == 0, "Complex path normalization should succeed");
	TEST_ASSERT(strcmp(path.String(), "/home/user/downloads/file.txt") == 0, 
		"Complex path should normalize correctly");
}

void test_normalize_multiple_parent_refs()
{
	Path path("/a/b/c/d/../../e", NULL, true);
	TEST_ASSERT(path.InitCheck() == 0, "Multiple parent references should normalize");
	TEST_ASSERT(strcmp(path.String(), "/a/b/e") == 0, "Parent references should be resolved");
}

// ============================================================================
// Normalization Tests - Relative Paths
// ============================================================================

void test_relative_path_forced_normalization()
{
	// Relative paths should be normalized to absolute with realpath
	Path path("./file.txt", NULL, true);
	TEST_ASSERT(path.InitCheck() == 0, "Relative path with normalization should resolve to absolute");
	// Note: Can't assert exact path since it depends on current working directory
	TEST_ASSERT(path.String()[0] == '/', "Normalized relative path should become absolute");
}

void test_relative_path_implicit_normalization()
{
	// Relative paths should be automatically normalized even without flag
	Path path("test/file.txt");
	TEST_ASSERT(path.InitCheck() == 0, "Relative path should auto-normalize");
	TEST_ASSERT(path.String()[0] == '/', "Auto-normalized relative path should become absolute");
}

// ============================================================================
// SetTo Method Tests
// ============================================================================

void test_setto_basic()
{
	Path path;
	status_t result = path.SetTo("/home/user");
	TEST_ASSERT(result == 0, "SetTo should return B_OK for valid path");
	TEST_ASSERT(strcmp(path.String(), "/home/user") == 0, "SetTo should set path correctly");
}

void test_setto_with_leaf()
{
	Path path;
	status_t result = path.SetTo("/home/user", "documents");
	TEST_ASSERT(result == 0, "SetTo with leaf should succeed");
	TEST_ASSERT(strcmp(path.String(), "/home/user/documents") == 0, "SetTo should combine path and leaf");
}

void test_setto_null_path()
{
	Path path("/tmp");
	status_t result = path.SetTo(NULL);
	TEST_ASSERT(result != 0, "SetTo with NULL path should fail");
	TEST_ASSERT(path.InitCheck() != 0, "Path should be uninitialized after failed SetTo");
}

void test_setto_absolute_leaf()
{
	Path path;
	status_t result = path.SetTo("/home", "/absolute");
	TEST_ASSERT(result != 0, "SetTo with absolute leaf should fail");
}

void test_setto_overwrites_previous()
{
	Path path("/tmp");
	status_t result = path.SetTo("/home/user");
	TEST_ASSERT(result == 0, "SetTo should succeed");
	TEST_ASSERT(strcmp(path.String(), "/home/user") == 0, "SetTo should overwrite previous path");
}

// ============================================================================
// Append Method Tests
// ============================================================================

void test_append_basic()
{
	Path path("/home");
	status_t result = path.Append("user");
	TEST_ASSERT(result == 0, "Append should succeed");
	TEST_ASSERT(strcmp(path.String(), "/home/user") == 0, "Append should add relative path");
}

void test_append_to_uninitialized()
{
	Path path;
	status_t result = path.Append("user");
	TEST_ASSERT(result != 0, "Append to uninitialized path should fail");
}

void test_append_with_leading_slash()
{
	Path path("/home");
	status_t result = path.Append("/user");
	TEST_ASSERT(result == 0, "Append with normalization should succeed");
	// With normalization, this might be handled differently
}

void test_append_with_normalization()
{
	Path path("/home/user");
	status_t result = path.Append("../documents", true);
	TEST_ASSERT(result == 0, "Append with normalization should succeed");
	TEST_ASSERT(strcmp(path.String(), "/home/documents") == 0, "Append with normalization should resolve ..");
}

// ============================================================================
// Leaf Method Tests
// ============================================================================

void test_leaf_simple()
{
	Path path("/home/user/file.txt");
	const char* leaf = path.Leaf();
	TEST_ASSERT(leaf != NULL, "Leaf should not be NULL for valid path");
	TEST_ASSERT(strcmp(leaf, "file.txt") == 0, "Leaf should return filename");
}

void test_leaf_root()
{
	Path path("/");
	const char* leaf = path.Leaf();
	TEST_ASSERT(leaf != NULL, "Leaf of root should not be NULL");
	TEST_ASSERT(strcmp(leaf, "") == 0, "Leaf of root should be empty string");
}

void test_leaf_single_component()
{
	Path path("/home");
	const char* leaf = path.Leaf();
	TEST_ASSERT(leaf != NULL, "Leaf should not be NULL");
	TEST_ASSERT(strcmp(leaf, "home") == 0, "Leaf should return single component");
}

void test_leaf_uninitialized()
{
	Path path;
	const char* leaf = path.Leaf();
	TEST_ASSERT(leaf == NULL, "Leaf of uninitialized path should be NULL");
}

// ============================================================================
// GetParent Method Tests
// ============================================================================

void test_getparent_simple()
{
	Path path("/home/user/file.txt");
	Path parent;
	status_t result = path.GetParent(&parent);
	TEST_ASSERT(result == 0, "GetParent should succeed");
	TEST_ASSERT(strcmp(parent.String(), "/home/user") == 0, "Parent should be correct");
}

void test_getparent_two_components()
{
	Path path("/home/user");
	Path parent;
	status_t result = path.GetParent(&parent);
	TEST_ASSERT(result == 0, "GetParent should succeed");
	TEST_ASSERT(strcmp(parent.String(), "/home") == 0, "Parent should be /home");
}

void test_getparent_root()
{
	Path path("/");
	Path parent;
	status_t result = path.GetParent(&parent);
	TEST_ASSERT(result != 0, "GetParent of root should fail");
}

void test_getparent_single_component()
{
	Path path("/home");
	Path parent;
	status_t result = path.GetParent(&parent);
	TEST_ASSERT(result == 0, "GetParent should succeed");
	TEST_ASSERT(strcmp(parent.String(), "/") == 0, "Parent of single component should be root");
}

// ============================================================================
// Comparison Operator Tests
// ============================================================================

void test_operator_equal_paths()
{
	Path path1("/home/user");
	Path path2("/home/user");
	TEST_ASSERT(path1 == path2, "Equal paths should compare equal");
}

void test_operator_unequal_paths()
{
	Path path1("/home/user");
	Path path2("/tmp");
	TEST_ASSERT(path1 != path2, "Different paths should not be equal");
}

void test_operator_compare_with_cstring()
{
	Path path("/home/user");
	TEST_ASSERT(path == "/home/user", "Path should equal matching C string");
	TEST_ASSERT(path != "/tmp", "Path should not equal different C string");
}

void test_operator_uninitialized_equal()
{
	Path path1;
	Path path2;
	TEST_ASSERT(path1 == path2, "Uninitialized paths should be equal");
}

void test_operator_normalized_paths_not_equal()
{
	// Note: Comparison is string-wise, not semantic
	Path path1("/home/user/./file");
	Path path2("/home/user/file");
	// These won't be equal without normalization because they have different strings
	// This is expected behavior per the documentation
}

// ============================================================================
// Assignment Operator Tests
// ============================================================================

void test_assignment_operator_path()
{
	Path path1("/home/user");
	Path path2;
	path2 = path1;
	TEST_ASSERT(strcmp(path2.String(), path1.String()) == 0, "Path assignment should copy path");
}

void test_assignment_operator_cstring()
{
	Path path;
	path = "/home/user";
	TEST_ASSERT(strcmp(path.String(), "/home/user") == 0, "C string assignment should work");
}

void test_assignment_operator_null()
{
	Path path("/home/user");
	path = NULL;
	TEST_ASSERT(path.InitCheck() != 0, "NULL assignment should unset path");
}

void test_assignment_operator_self()
{
	Path path("/home/user");
	path = path;
	TEST_ASSERT(strcmp(path.String(), "/home/user") == 0, "Self assignment should be safe");
}

// ============================================================================
// Unset Method Tests
// ============================================================================

void test_unset()
{
	Path path("/home/user");
	path.Unset();
	TEST_ASSERT(path.InitCheck() != 0, "Unset should make path uninitialized");
	TEST_ASSERT(path.String() == NULL, "Unset path should have NULL string");
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main(int argc, char** argv)
{
	printf("=== Path Class Test Suite ===\n\n");

	printf("--- Basic Constructor and Initialization Tests ---\n");
	test_default_constructor();
	test_constructor_with_absolute_path();
	test_constructor_with_leaf();
	test_copy_constructor();

	printf("\n--- Normalization Tests - Trailing Slashes ---\n");
	test_normalize_trailing_slash();
	test_root_path_no_normalization_needed();

	printf("\n--- Normalization Tests - Double Slashes ---\n");
	test_normalize_double_slash();
	test_normalize_multiple_double_slashes();
	test_normalize_consecutive_slashes();

	printf("\n--- Normalization Tests - Dot and Double Dot ---\n");
	test_normalize_single_dot();
	test_normalize_double_dot();
	test_normalize_multiple_dots();
	test_normalize_dot_at_end();
	test_normalize_double_dot_at_end();

	printf("\n--- Normalization Tests - Complex Cases ---\n");
	test_normalize_complex_path();
	test_normalize_multiple_parent_refs();

	printf("\n--- Normalization Tests - Relative Paths ---\n");
	test_relative_path_forced_normalization();
	test_relative_path_implicit_normalization();

	printf("\n--- SetTo Method Tests ---\n");
	test_setto_basic();
	test_setto_with_leaf();
	test_setto_null_path();
	test_setto_absolute_leaf();
	test_setto_overwrites_previous();

	printf("\n--- Append Method Tests ---\n");
	test_append_basic();
	test_append_to_uninitialized();
	test_append_with_leading_slash();
	test_append_with_normalization();

	printf("\n--- Leaf Method Tests ---\n");
	test_leaf_simple();
	test_leaf_root();
	test_leaf_single_component();
	test_leaf_uninitialized();

	printf("\n--- GetParent Method Tests ---\n");
	test_getparent_simple();
	test_getparent_two_components();
	test_getparent_root();
	test_getparent_single_component();

	printf("\n--- Comparison Operator Tests ---\n");
	test_operator_equal_paths();
	test_operator_unequal_paths();
	test_operator_compare_with_cstring();
	test_operator_uninitialized_equal();
	test_operator_normalized_paths_not_equal();

	printf("\n--- Assignment Operator Tests ---\n");
	test_assignment_operator_path();
	test_assignment_operator_cstring();
	test_assignment_operator_null();
	test_assignment_operator_self();

	printf("\n--- Unset Method Tests ---\n");
	test_unset();

	printf("\n=== Test Summary ===\n");
	printf("Total Tests: %d\n", g_testCount);
	printf("Passed: %d\n", g_testCount - g_failureCount);
	printf("Failed: %d\n", g_failureCount);

	return g_failureCount > 0 ? 1 : 0;
}
