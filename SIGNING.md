# Code Signing Guide for TilePad

## Azure Trusted Signing Setup

### Account Details

- **Subscription**: Azure subscription 1 (`5aa3cfb0-a93d-475d-90ee-d0362675f6c0`)
- **Resource Group**: `TilePad-Signing`
- **Signing Account**: `dynart-signing`
- **Account URI**: `https://weu.codesigning.azure.net/`
- **Region**: West Europe
- **User**: `z3r0c00l@dynart.net`

### Certificate Profiles

- **dynart-private** — PrivateTrust (for testing, not trusted by Windows publicly)
- **dynart-public** — PublicTrust (create once Public identity validation is approved)

### Roles Assigned

- Artifact Signing Identity Verifier
- Artifact Signing Certificate Profile Signer

### Prerequisites

- Azure CLI: `C:\Program Files\Microsoft SDKs\Azure\CLI2\wbin\az.cmd`
- TrustedSigning PowerShell module: `Install-Module -Name TrustedSigning -Scope CurrentUser`
- Inno Setup 6: `C:\Program Files (x86)\Inno Setup 6\ISCC.exe`

### Signing Tools (auto-installed by TrustedSigning module)

- signtool.exe: `C:\Users\stan_\AppData\Local\TrustedSigning\Microsoft.Windows.SDK.BuildTools\...\signtool.exe`
- Dlib: `C:\Users\stan_\AppData\Local\TrustedSigning\Microsoft.Trusted.Signing.Client\...\Azure.CodeSigning.Dlib.dll`
- Metadata: `C:\Users\stan_\AppData\Local\TrustedSigning\Microsoft.Trusted.Signing.Client\...\metadata.json`

**Important**: Azure CLI must be in PATH for signtool to authenticate. Add this at the start of your session:

```powershell
$env:PATH = "C:\Program Files\Microsoft SDKs\Azure\CLI2\wbin;" + $env:PATH
```

---

## Signing a File

### Step 1: Log in to Azure

```powershell
& "C:\Program Files\Microsoft SDKs\Azure\CLI2\wbin\az.cmd" login
```

### Step 2: Sign with signtool

```powershell
$env:PATH = "C:\Program Files\Microsoft SDKs\Azure\CLI2\wbin;" + $env:PATH

$signtool = "C:\Users\stan_\AppData\Local\TrustedSigning\Microsoft.Windows.SDK.BuildTools\Microsoft.Windows.SDK.BuildTools.10.0.26100.4188\bin\10.0.26100.0\x64\signtool.exe"
$dlib = "C:\Users\stan_\AppData\Local\TrustedSigning\Microsoft.Trusted.Signing.Client\Microsoft.Trusted.Signing.Client.1.0.95\bin\x64\Azure.CodeSigning.Dlib.dll"
$metadata = "C:\Users\stan_\AppData\Local\TrustedSigning\Microsoft.Trusted.Signing.Client\Microsoft.Trusted.Signing.Client.1.0.95\bin\x64\metadata.json"

& $signtool sign /v /fd SHA256 /tr "http://timestamp.acs.microsoft.com" /td SHA256 /dlib $dlib /dmdf $metadata "C:\Projects\TilePad\_package\TilePad.exe"
```

**Note**: The metadata.json contains the endpoint, account name, and certificate profile name. Edit it to switch profiles:

```json
{
  "Endpoint": "https://weu.codesigning.azure.net/",
  "CodeSigningAccountName": "dynart-signing",
  "CertificateProfileName": "dynart-private",
  "ExcludeCredentials": [
    "ManagedIdentityCredential",
    "SharedTokenCacheCredential",
    "VisualStudioCredential",
    "VisualStudioCodeCredential",
    "InteractiveBrowserCredential",
    "EnvironmentCredential",
    "WorkloadIdentityCredential"
  ]
}
```

---

## Once Public Identity Validation is Completed

### Create Public Trust Certificate Profile

1. Get the identity validation ID from Azure Portal > dynart-signing > Identity validations
2. Run:

```powershell
& "C:\Program Files\Microsoft SDKs\Azure\CLI2\wbin\az.cmd" trustedsigning certificate-profile create `
  --resource-group TilePad-Signing `
  --account-name dynart-signing `
  --profile-name dynart-public `
  --profile-type PublicTrust `
  --identity-validation-id "<YOUR_PUBLIC_IDENTITY_VALIDATION_ID>" `
  --include-street-address false `
  --include-postal-code false `
  -o table
```

3. Update metadata.json: change `CertificateProfileName` from `dynart-private` to `dynart-public`

---

## Full Build, Sign & Release

```powershell
# 0. Ensure Azure CLI is in PATH and logged in
$env:PATH = "C:\Program Files\Microsoft SDKs\Azure\CLI2\wbin;" + $env:PATH
& "C:\Program Files\Microsoft SDKs\Azure\CLI2\wbin\az.cmd" login

# 1. Build
cd C:\Projects\TilePad\build
C:\qt\Qt6\Tools\CMake_64\bin\cmake.exe --build . --config Release

# 2. Deploy Qt DLLs
$env:PATH = "C:\qt\Qt6\6.10.2\mingw_64\bin;C:\qt\Qt6\Tools\mingw1310_64\bin;" + $env:PATH
C:\qt\Qt6\6.10.2\mingw_64\bin\windeployqt6.exe TilePad.exe

# 3. Copy to _package
Copy-Item TilePad.exe, *.dll -Destination ..\\_package -Force
Copy-Item platforms\* -Destination ..\\_package\\platforms -Force -Recurse
Copy-Item styles\* -Destination ..\\_package\\styles -Force -Recurse
Copy-Item imageformats\* -Destination ..\\_package\\imageformats -Force -Recurse
Copy-Item iconengines\* -Destination ..\\_package\\iconengines -Force -Recurse
Copy-Item generic\* -Destination ..\\_package\\generic -Force -Recurse
Copy-Item tls\* -Destination ..\\_package\\tls -Force -Recurse
Copy-Item translations\* -Destination ..\\_package\\translations -Force -Recurse

# 4. Sign the exe
$signtool = "C:\Users\stan_\AppData\Local\TrustedSigning\Microsoft.Windows.SDK.BuildTools\Microsoft.Windows.SDK.BuildTools.10.0.26100.4188\bin\10.0.26100.0\x64\signtool.exe"
$dlib = "C:\Users\stan_\AppData\Local\TrustedSigning\Microsoft.Trusted.Signing.Client\Microsoft.Trusted.Signing.Client.1.0.95\bin\x64\Azure.CodeSigning.Dlib.dll"
$metadata = "C:\Users\stan_\AppData\Local\TrustedSigning\Microsoft.Trusted.Signing.Client\Microsoft.Trusted.Signing.Client.1.0.95\bin\x64\metadata.json"
& $signtool sign /v /fd SHA256 /tr "http://timestamp.acs.microsoft.com" /td SHA256 /dlib $dlib /dmdf $metadata ..\\_package\\TilePad.exe

# 5. Build installer
cd C:\Projects\TilePad
& "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" setup.iss

# 6. Sign installer
& $signtool sign /v /fd SHA256 /tr "http://timestamp.acs.microsoft.com" /td SHA256 /dlib $dlib /dmdf $metadata Output\\tilepad-setup-0.5.0.exe

# 7. Upload to GitHub release
gh release upload v0.5.0 Output\\tilepad-setup-0.5.0.exe --clobber
```
