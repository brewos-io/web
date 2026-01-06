# BrewOS Website

Marketing and documentation website for the BrewOS project. Built with [Astro](https://astro.build/) and deployed to GitHub Pages.

## Development

```bash
npm install
npm run dev
```

The site will be available at `http://localhost:4321`

## Build

```bash
npm run build
```

Build output will be in the `dist/` directory.

## Deployment

The site is automatically deployed to GitHub Pages on pushes to `main` branch via GitHub Actions.

## Project Structure

```
.
├── src/
│   ├── components/     # Astro components
│   ├── layouts/        # Page layouts
│   ├── pages/          # Route pages
│   └── styles/         # Global styles
├── public/             # Static assets (copied during build)
│   └── assets/         # Symlink to root assets folder (../../assets)
└── astro.config.mjs    # Astro configuration
```

## Related Repositories

- [Wiki](https://github.com/brewos-io/wiki) - Complete user documentation and guides
- [Firmware](https://github.com/brewos-io/firmware) - Main firmware repository (ESP32, Pico, web UI, cloud)
- [Home Assistant Integration](https://github.com/brewos-io/homeassistant) - HA custom component and Lovelace card

## License

Licensed under the Apache License 2.0 with Commons Clause - see [LICENSE](LICENSE) file for details.
