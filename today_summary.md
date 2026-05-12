# 2026-04-30 Session Summary

## 1. Compiler Theory Quiz Answer Key
Created a complete DOCX document (`quiz_answers.docx`) with solutions and explanations:
- **Q1:** d) Optimization (Compiler pipeline stages)
- **Q2:** c) C (Class environment stores method signatures)
- **Q3:** c) The occurrence of y (Bound variable in a Let binding)
- **Q4:** NFA/LR items for Viable Prefixes

## 2. K8s Cluster Access
- Installed `kubectl` and configured it with the mounted kubeconfig.
- Connected to the `ai-agents` namespace.
- Verified access to all services (Hermes, Postgres, merged-backend, etc.).

## 3. Hermes vs. OpenClaw
- Discussed that Hermes is older (July 2025) than OpenClaw (Nov 2025).
- Noted Hermes has a more engineering-first architecture (transport abstractions, TUI rewrite).

## 4. RAG Server Deployment
- Built a FastMCP-based RAG bridge connecting Hermes/OpenCode to your local infrastructure.
- **Stack:** pgvector + Qwen3 Embeddings.
- **Deployment:** Created a ConfigMap and Deployment for `rag-mcp-server` in the `ai-agents` namespace.
- **Current State:** The pod is deployed and running, but the service is inactive because the `dulc3-top` node (hosting the vector models) was turned off to save energy.

## 5. Action Items
- [ ] Turn on `dulc3-top` node to reactivate vector models.
- [ ] Test the `rag-mcp-server` connectivity to Postgres and the Embedding backend.
- [ ] Register the RAG server with Hermes as a native MCP tool.
