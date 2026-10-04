# Security

Continuum-LOB is a research simulator. Treat untrusted configuration files,
serialized model artifacts, and external datasets with care:

- Validate YAML/JSON schema before applying parameters.
- Do not load untrusted binary checkpoints into privileged processes.
- Dependencies should be pinned when packaging for distribution.
