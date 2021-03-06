#include <mitsuba/core/distr_1d.h>
#include <mitsuba/core/properties.h>
#include <mitsuba/core/warp.h>
#include <mitsuba/render/phase.h>

/**!

.. _phase-rayleigh:

Rayleigh phase function (:monosp:`rayleigh`)
-----------------------------------------------

Scattering by particles that are much smaller than the wavelength
of light (e.g. individual molecules in the atmosphere) is well-approximated
by the Rayleigh phase function. This plugin implements an unpolarized
version of this scattering model (*i.e.* the effects of polarization are
ignored). This plugin is useful for simulating scattering in planetary
atmospheres.

This model has no parameters.

*/

NAMESPACE_BEGIN(mitsuba)

template <typename Float, typename Spectrum>
class RayleighPhaseFunction final : public PhaseFunction<Float, Spectrum> {
public:
    MTS_IMPORT_BASE(PhaseFunction, m_flags, m_components)
    MTS_IMPORT_TYPES(PhaseFunctionContext)

    RayleighPhaseFunction(const Properties &props) : Base(props) {
        if constexpr (is_polarized_v<Spectrum>)
            Log(Warn,
                "Polarized version of Rayleigh phase function not implemented, "
                "falling back to scalar version");

        m_flags = +PhaseFunctionFlags::Anisotropic;
        m_components.push_back(m_flags);
    }

    MTS_INLINE Float eval_rayleigh(Float cos_theta) const {
        return (3.f / 16.f) * math::InvPi<Float> *
               (1.f + enoki::sqr(cos_theta));
    }

    std::pair<Vector3f, Float> sample(const PhaseFunctionContext & /* ctx */,
                                      const MediumInteraction3f &mi,
                                      Float /* sample1 */,
                                      const Point2f &sample2,
                                      Mask active) const override {
        MTS_MASKED_FUNCTION(ProfilerPhase::PhaseFunctionSample, active);

        Float z         = 2.f * (2.f * sample2.x() - 1.f);
        Float tmp       = enoki::sqrt(enoki::sqr(z) + 1.f);
        Float A         = enoki::cbrt(z + tmp);
        Float B         = enoki::cbrt(z - tmp);
        Float cos_theta = A + B;
        Float sin_theta = enoki::safe_sqrt(1.0f - enoki::sqr(cos_theta));
        auto [sin_phi, cos_phi] = enoki::sincos(math::TwoPi<Float> * sample2.y());

        auto wo = Vector3f{ sin_theta * cos_phi, sin_theta * sin_phi, cos_theta };

        wo        = mi.to_world(wo);
        Float pdf = eval_rayleigh(-cos_theta);
        return { wo, pdf };
    }

    Float eval(const PhaseFunctionContext & /* ctx */,
               const MediumInteraction3f &mi, const Vector3f &wo,
               Mask active) const override {
        MTS_MASKED_FUNCTION(ProfilerPhase::PhaseFunctionEvaluate, active);
        return eval_rayleigh(dot(wo, mi.wi));
    }

    std::string to_string() const override { return "RayleighPhaseFunction[]"; }

    MTS_DECLARE_CLASS()

private:
};

MTS_IMPLEMENT_CLASS_VARIANT(RayleighPhaseFunction, PhaseFunction)
MTS_EXPORT_PLUGIN(RayleighPhaseFunction, "Rayleigh phase function")
NAMESPACE_END(mitsuba)
