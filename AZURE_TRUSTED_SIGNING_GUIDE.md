# Azure Trusted Signing — Complete Setup Guide

A step-by-step guide for setting up Azure Trusted Signing for any Windows application. Covers everything from Azure account setup to signing executables and installers.

---

## Table of Contents

1. [Prerequisites](#1-prerequisites)
2. [Azure Account & Resource Setup](#2-azure-account--resource-setup)
3. [Identity Validation](#3-identity-validation)
4. [Certificate Profiles](#4-certificate-profiles)
5. [Role Assignments](#5-role-assignments)
6. [Install Signing Tools](#6-install-signing-tools)
7. [Configure metadata.json](#7-configure-metadatajson)
8. [Sign a File](#8-sign-a-file)
9. [Full Build, Sign & Release Workflow](#9-full-build-sign--release-workflow)
10. [Troubleshooting](#10-troubleshooting)

---

## 1. Prerequisites

Install these before starting:

| Tool | Install Command | Purpose |
|------|----------------|---------|
| **Azure CLI** | `winget install Microsoft.AzureCLI` | Authenticate with Azure |
| **TrustedSigning PowerShell module** | `Install-Module -Name TrustedSigning -Scope CurrentUser` | Installs signtool, dlib, and metadata tools |

After installing Azure CLI, note its location:
```
C:\Program Files\Microsoft SDKs\Azure\CLI2\wbin\az.cmd
```

**Critical**: Azure CLI must be in your PATH for the signing dlib to authenticate. Always run this at the start of your session:
```powershell
$env:PATH = "C:\Program Files\Microsoft SDKs\Azure\CLI2\wbin;" + $env:PATH
```

---

## 2. Azure Account & Resource Setup

### 2.1 Log in to Azure

```powershell
& "C:\Program Files\Microsoft SDKs\Azure\CLI2\wbin\az.cmd" login
```

### 2.2 Register the CodeSigning provider (once per subscription)

```powershell
az provider register --namespace Microsoft.CodeSigning
```

Wait for it to complete (~1-2 minutes):
```powershell
az provider show --namespace Microsoft.CodeSigning --query "registrationState"
# Should return "Registered"
```

### 2.3 Create a Resource Group

```powershell
az group create --name <ResourceGroup> --location westeurope
```

Choose a region that supports Trusted Signing: `westeurope`, `eastus`, `southcentralus`, `westus3`, etc.

### 2.4 Install the Trusted Signing CLI extension

```powershell
az extension add --name trustedsigning
```

### 2.5 Create a Trusted Signing Account

```powershell
az trustedsigning create `
  --resource-group <ResourceGroup> `
  --account-name <AccountName> `
  --location westeurope `
  --sku Basic
```

Note the **Account URI** — it follows the pattern: `https://<region>.codesigning.azure.net/`

Example: `https://weu.codesigning.azure.net/` for West Europe.

---

## 3. Identity Validation

Identity validation is done through the **Azure Portal** (not CLI). There are two types:

| Type | Purpose | Approval Time | Certificate Shows |
|------|---------|---------------|-------------------|
| **Private** | Testing only, not publicly trusted | Instant (auto-approved) | Organization's tenant name |
| **Public** | Production, trusted by Windows | 1-7 business days (manual review by Microsoft) | Your verified organization name |

### 3.1 Create an Identity Validation

1. Go to **Azure Portal** > your Trusted Signing account > **Identity validations**
2. Click **+ New identity validation**
3. Fill in your organization details
4. For **Public**: provide legal business name, address, and supporting documents
5. For **Private**: auto-completes immediately

### 3.2 Wait for Approval

- **Private**: Ready immediately
- **Public**: Microsoft reviews manually. Check status in the portal. You'll need the **Identity Validation ID** (a GUID) once it shows "Completed"

**Important**: You need **separate** identity validations for Private and Public profiles. A Private validation ID cannot be used to create a Public profile (and vice versa).

---

## 4. Certificate Profiles

A certificate profile links your signing account to an identity validation. You need one profile per trust level.

### 4.1 Create a Profile

```powershell
az trustedsigning certificate-profile create `
  --resource-group <ResourceGroup> `
  --account-name <AccountName> `
  --profile-name <ProfileName> `
  --profile-type <PublicTrust|PrivateTrust> `
  --identity-validation-id "<GUID>" `
  --include-street-address false `
  --include-postal-code false `
  -o table
```

**Example** (Public Trust):
```powershell
az trustedsigning certificate-profile create `
  --resource-group MyApp-Signing `
  --account-name mycompany-signing `
  --profile-name mycompany-public `
  --profile-type PublicTrust `
  --identity-validation-id "aa41670d-630b-4f5b-a75f-3baf302ee867" `
  --include-street-address false `
  --include-postal-code false `
  -o table
```

### 4.2 List Existing Profiles

```powershell
az trustedsigning certificate-profile list `
  --resource-group <ResourceGroup> `
  --account-name <AccountName> `
  -o table
```

### 4.3 Recommended Naming Convention

- `<company>-public` — PublicTrust (production)
- `<company>-private` — PrivateTrust (testing)

---

## 5. Role Assignments

Your Azure user needs two roles on the signing account:

1. **Artifact Signing Identity Verifier**
2. **Artifact Signing Certificate Profile Signer**

### 5.1 Get Your User's Object ID

```powershell
$objectId = (az ad signed-in-user show --query id -o tsv)
```

### 5.2 Get the Signing Account Resource ID

```powershell
$resourceId = (az trustedsigning show `
  --resource-group <ResourceGroup> `
  --name <AccountName> `
  --query id -o tsv)
```

### 5.3 Assign Both Roles

```powershell
az role assignment create --assignee $objectId --role "Trusted Signing Identity Verifier" --scope $resourceId
az role assignment create --assignee $objectId --role "Trusted Signing Certificate Profile Signer" --scope $resourceId
```

**Note**: The role names in Azure may appear as "Trusted Signing Identity Verifier" or "Artifact Signing Identity Verifier" depending on your Azure CLI version. If one fails, try the other prefix.

---

## 6. Install Signing Tools

The TrustedSigning PowerShell module auto-installs the required tools on first use. You can trigger this by running:

```powershell
Invoke-TrustedSigning -Help
```

Or just run a signing command — it will download the packages automatically.

The tools are installed to:
```
C:\Users\<USER>\AppData\Local\TrustedSigning\
├── Microsoft.Windows.SDK.BuildTools\...\signtool.exe
└── Microsoft.Trusted.Signing.Client\...\
    ├── Azure.CodeSigning.Dlib.dll
    └── metadata.json
```

### 6.1 Locate the Exact Paths

After installation, find the paths:
```powershell
$base = "$env:LOCALAPPDATA\TrustedSigning"

$signtool = Get-ChildItem -Path $base -Recurse -Filter "signtool.exe" |
    Where-Object { $_.FullName -match "x64" } |
    Select-Object -First 1 -ExpandProperty FullName

$dlib = Get-ChildItem -Path $base -Recurse -Filter "Azure.CodeSigning.Dlib.dll" |
    Where-Object { $_.FullName -match "x64" } |
    Select-Object -First 1 -ExpandProperty FullName

$metadata = Get-ChildItem -Path $base -Recurse -Filter "metadata.json" |
    Where-Object { $_.FullName -match "x64" } |
    Select-Object -First 1 -ExpandProperty FullName

Write-Host "signtool: $signtool"
Write-Host "dlib:     $dlib"
Write-Host "metadata: $metadata"
```

Save these paths — you'll need them for every signing command.

---

## 7. Configure metadata.json

The metadata.json file tells the signing dlib which account and profile to use. Edit it at the path found above:

```json
{
  "Endpoint": "https://weu.codesigning.azure.net/",
  "CodeSigningAccountName": "<AccountName>",
  "CertificateProfileName": "<ProfileName>",
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

### Key Fields

| Field | Value |
|-------|-------|
| `Endpoint` | `https://weu.codesigning.azure.net/` (West Europe) or `https://eus.codesigning.azure.net/` (East US), etc. |
| `CodeSigningAccountName` | Your signing account name |
| `CertificateProfileName` | The profile to sign with (e.g., `mycompany-public`) |

### Why ExcludeCredentials?

The `ExcludeCredentials` list forces the dlib to use **Azure CLI credentials only**. Without this, it tries other credential methods (Managed Identity, Visual Studio, etc.) which fail on local machines and cause timeouts or hangs.

**This is the #1 cause of signing failures on developer machines.** Always include this list.

---

## 8. Sign a File

### 8.1 Ensure Azure CLI is Logged In

```powershell
$env:PATH = "C:\Program Files\Microsoft SDKs\Azure\CLI2\wbin;" + $env:PATH
& "C:\Program Files\Microsoft SDKs\Azure\CLI2\wbin\az.cmd" login
```

### 8.2 Sign

```powershell
$signtool = "<path-to-signtool.exe>"
$dlib = "<path-to-Azure.CodeSigning.Dlib.dll>"
$metadata = "<path-to-metadata.json>"

& $signtool sign /v /fd SHA256 /tr "http://timestamp.acs.microsoft.com" /td SHA256 /dlib $dlib /dmdf $metadata "<FILE_TO_SIGN>"
```

### 8.3 Verify Signature

Right-click the signed file > Properties > Digital Signatures tab. You should see:
- **Name of signer**: Your organization name (for PublicTrust) or tenant name (for PrivateTrust)
- **Digest algorithm**: sha256
- **Timestamp**: Present (from `timestamp.acs.microsoft.com`)

---

## 9. Full Build, Sign & Release Workflow

Template for any application:

```powershell
# 0. Setup
$env:PATH = "C:\Program Files\Microsoft SDKs\Azure\CLI2\wbin;" + $env:PATH
& "C:\Program Files\Microsoft SDKs\Azure\CLI2\wbin\az.cmd" login

$signtool = "<path-to-signtool.exe>"
$dlib = "<path-to-Azure.CodeSigning.Dlib.dll>"
$metadata = "<path-to-metadata.json>"

# 1. Build your application
# ... (your build commands here)

# 2. Sign the executable(s)
& $signtool sign /v /fd SHA256 /tr "http://timestamp.acs.microsoft.com" /td SHA256 /dlib $dlib /dmdf $metadata "path\to\YourApp.exe"

# 3. Build installer (if applicable)
# ... (Inno Setup, NSIS, WiX, etc.)

# 4. Sign the installer
& $signtool sign /v /fd SHA256 /tr "http://timestamp.acs.microsoft.com" /td SHA256 /dlib $dlib /dmdf $metadata "path\to\your-setup.exe"

# 5. Upload to release
gh release upload v1.0.0 "path\to\your-setup.exe" --clobber
```

### Signing Multiple Files

You can sign multiple files in one command:
```powershell
& $signtool sign /v /fd SHA256 /tr "http://timestamp.acs.microsoft.com" /td SHA256 /dlib $dlib /dmdf $metadata "file1.exe" "file2.dll" "file3.exe"
```

---

## 10. Troubleshooting

### "SignTool failed with exit code" / Timeout

**Cause**: The dlib can't find Azure CLI to authenticate.

**Fix**: Ensure Azure CLI is in PATH:
```powershell
$env:PATH = "C:\Program Files\Microsoft SDKs\Azure\CLI2\wbin;" + $env:PATH
```

And ensure `ExcludeCredentials` is set in metadata.json (see section 7).

### 403 Forbidden

**Cause**: One of:
- Certificate profile doesn't exist (check name in metadata.json)
- Wrong identity validation type (Private ID used for Public profile)
- Missing role assignments

**Fix**:
1. List profiles: `az trustedsigning certificate-profile list ...`
2. Verify the profile name in metadata.json matches exactly
3. Check role assignments (section 5)

### "identity validation details not found for Tenant"

**Cause**: Trying to create a PublicTrust profile with a Private identity validation ID (or vice versa).

**Fix**: Use the correct identity validation ID. Private and Public validations are separate — each has its own GUID.

### "Permission denied" when linking/building after signing

**Cause**: A previous signtool process is still running or the file is locked.

**Fix**:
```powershell
Stop-Process -Name signtool -Force -ErrorAction SilentlyContinue
Stop-Process -Name YourApp -Force -ErrorAction SilentlyContinue
```

### Signature shows "Unknown Publisher"

**Cause**: Signed with PrivateTrust profile (not publicly trusted).

**Fix**: Switch to PublicTrust profile in metadata.json. You must have a completed Public identity validation.

### AzureSignTool doesn't work

**AzureSignTool is for Azure Key Vault, NOT Trusted Signing.** They are different services. For Trusted Signing, use `signtool.exe` with the `Azure.CodeSigning.Dlib.dll` as documented above.

---

## Quick Reference

### Signing Command Template
```powershell
& $signtool sign /v /fd SHA256 /tr "http://timestamp.acs.microsoft.com" /td SHA256 /dlib $dlib /dmdf $metadata "<FILE>"
```

### Signing Command Flags
| Flag | Meaning |
|------|---------|
| `/v` | Verbose output |
| `/fd SHA256` | File digest algorithm |
| `/tr <URL>` | Timestamp server URL |
| `/td SHA256` | Timestamp digest algorithm |
| `/dlib <path>` | Path to Azure.CodeSigning.Dlib.dll |
| `/dmdf <path>` | Path to metadata.json |

### Useful Azure CLI Commands
```powershell
# List signing accounts
az trustedsigning list --resource-group <RG> -o table

# List certificate profiles
az trustedsigning certificate-profile list --resource-group <RG> --account-name <Account> -o table

# Show account details
az trustedsigning show --resource-group <RG> --name <Account> -o table
```
