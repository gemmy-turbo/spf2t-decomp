# Source origins

Recovered functional headers originate from the USA960620 reconstruction.
Source comments identify original addresses; these are a partial reconstruction,
not a release of Capcom's original source or an automatic legal-clearance claim.

Classic CPU and selection use Tommy McLeroy's GPL-3.0-only source as a guide,
adapted and checked against USA instructions. Required notices remain in the
files. `third_party/cps2crypt` is copied from tmcleroy/cps2recomp at `145314f81e18d05040fcbe9e87c4df50a57e9a28`.
The cipher's MAME BSD notice and GPL entrypoint notice are retained. The import
helper adapts its key-layout contract and passes MAME's inclusive upper cipher
word to the exclusive endpoint explicitly. This imports user-supplied material;
it contains no board key. Sources are available at
https://github.com/tmcleroy/cps2recomp/tree/145314f81e18d05040fcbe9e87c4df50a57e9a28.
