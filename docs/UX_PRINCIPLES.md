# Dedus UI/UX direction

## Product truth
The browser app in `frontend/` is a clickable UX prototype, not a native interface running on the kernel. The ARM64 kernel currently exposes a serial shell. A native GUI requires graphics, input, scheduling, memory isolation and user-space services.

## Design language: solid morphism
- Use solid, tactile surfaces, crisp shadow outlines, restrained depth and consistent spacing/type scales.
- Avoid blanket glass blur. Translucency is a layer treatment, not the whole design language.
- Motion explains state changes, honors reduced-motion preferences and never blocks input.

## Core interaction model
1. Home/launcher: search, pinned apps and a small amount of glanceable information.
2. Quick settings and notifications: top-edge reachable, individually actionable and clearly separated.
3. Recents/task switcher: clear app identity, predictable dismissal and accessible controls.
4. Lock screen: readable time/status and honest privacy indicators.
5. Settings: searchable categories, visible state, explanations before destructive actions.
6. File manager: explicit locations and operations; destructive actions require reversible confirmation.
7. Terminal/developer tools: separate from everyday surfaces, with clear privilege and command context.

## Mobile ergonomics
- Design for narrow screens first; expand to larger devices without stretching every control.
- Aim for 48 dp minimum touch targets where practical.
- Every gesture must have a visible or accessible alternative.
- Respect keyboard, system bars, notches and gesture regions.

## Accessibility and performance
- Support text scaling, contrast, focus states, screen readers and reduced motion.
- Solid-color/software composition is the baseline. Enable blur, live wallpaper and complex effects only when frame-time and thermal budgets allow.
- Degrade visual effects before input responsiveness or text clarity.
- Display real system telemetry only after drivers/services provide it. Label simulated prototype values.

## Security and privacy UX
- No default root/admin passwords in distributed demos.
- Permission prompts explain the affected capability and appear at the moment of need.
- Privacy indicators reflect real camera/microphone/location activity.
- Secure boot, encryption, sandboxing and app verification must be implemented and tested before being advertised.

## Delivery stages
- Current: Dedus branding, prototype-only login disclaimer, kernel memory report and automated script checks.
- Next: responsive prototype, accessible interaction states and browser-level tests.
- Later: native compositor/window system after graphics/input drivers; share design tokens across native surfaces.
