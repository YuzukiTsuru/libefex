//! Basic smoke test

use libfex::Context;

#[test]
fn test_context_creation() {
    // Test if Context object can be successfully created and destroyed
    let result = Context::new();
    assert!(result.is_ok(), "Failed to create Context");
    
    // Context will be automatically destroyed here (via Drop trait)
}